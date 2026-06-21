# Windows PE Boot Process Reference

This document describes the full boot chain of Windows Preinstallation Environment (WinPE), starting from an ISO file through to the graphical command-prompt environment. Sources include Microsoft Learn, OSDeploy.com, and community documentation.

---

## 1. Overview of Files

| File | Role | Location (in WIM) |
|---|---|---|
| `bootmgr` / `bootmgfw.efi` | Boot manager (BIOS/UEFI) | `\boot\` on ISO media |
| `BCD` | Boot Configuration Data store | `\boot\` on ISO media |
| `boot.sdi` | RAM disk template (NTFS-formatted disk image) | `\boot\` on ISO media |
| `boot.wim` | Windows PE image file (contains the full OS payload) | `\sources\` on ISO media |
| `winload.exe` / `winload.efi` | OS loader (BIOS/UEFI) | `\windows\system32\boot\` |
| `ntoskrnl.exe` | Windows NT kernel | `\windows\system32\` |
| `smss.exe` | Session Manager Subsystem — first user-mode process | `\windows\system32\` |
| `winlogon.exe` | Logon manager (launches winpeshl.exe) | `\windows\system32\` |
| `winpeshl.exe` | WinPE shell (reads winpeshl.ini, falls back to startnet.cmd) | `\windows\system32\` |
| `startnet.cmd` | Default startup script; calls `wpeinit.exe` | `\windows\system32\` |
| `wpeinit.exe` | PnP init, network stack, unattend.xml processing | `\windows\system32\` |

---

## 2. Boot Chain — Step by Step

### Phase 1: Media Bootstrap

| Firmware Type | Boot Path |
|---|---|
| **BIOS (Legacy)** | ISO El Torito → bootsector → `bootmgr` |
| **UEFI** | ISO UDF/ISO9660 → FAT partition embedded in ISO → `\efi\boot\bootx64.efi` (which is `bootmgfw.efi`) |

### Phase 2: Boot Manager (bootmgr / bootmgfw.efi)

1. Reads `\boot\BCD` from the boot media.
2. Finds the **Windows Boot Loader** entry whose `device` is set to `ramdisk=[boot]\sources\boot.wim,{GUID}`.
3. Looks up the corresponding **Setup Ramdisk Options** entry (by GUID), which specifies:
   - `ramdisksdidevice`: `boot` (relative to boot media)
   - `ramdisksdipath`: `\boot\boot.sdi`

BCD excerpt (BIOS example):
```
Windows Boot Loader
-------------------
identifier              {default}
device                  ramdisk=[boot]\sources\boot.wim,{ramdiskoptions}
path                    \windows\system32\boot\winload.exe
description             Windows PE
osdevice                ramdisk=[boot]\sources\boot.wim,{ramdiskoptions}
systemroot              \windows
winpe                   Yes
detecthal               Yes

Setup Ramdisk Options
---------------------
identifier              {ramdiskoptions}
ramdisksdidevice        boot
ramdisksdipath          \boot\boot.sdi
```

### Phase 3: RAM Disk Creation

1. `bootmgr` loads `boot.sdi` into memory and uses it as a **physical disk template**.
   - `boot.sdi` is a tiny (~3 MB) pre-formatted NTFS disk image.
   - It provides the partition table, NTFS volume header, and minimal filesystem metadata.
2. `bootmgr` appends the contents of `boot.wim` as a contiguous file on this in-memory NTFS volume.
3. The result is a fully functional RAM-backed NTFS volume mounted as `X:` (the default WinPE system drive).

### Phase 4: OS Loader (winload.exe / winload.efi)

1. `bootmgr` locates the boot image marked with the `boot` flag inside `boot.wim` (only one image per WIM can be bootable).
2. It executes `\windows\system32\boot\winload.exe` (BIOS) or `winload.efi` (UEFI) **from the mounted WIM**.
3. `winload.exe`:
   - Loads the **Hardware Abstraction Layer** (`hal.dll`).
   - Loads the **SYSTEM registry hive** (`\windows\system32\config\system`).
   - Loads **BOOT_START drivers** (disk controllers, bus drivers, etc.).
   - Reads the `winpe` flag from BCD to set PE-specific behavior (e.g. skip pagefile, no crash dumps).
   - Transfers control to the NT kernel.

### Phase 5: Kernel (ntoskrnl.exe)

1. `ntoskrnl.exe` initializes the kernel executive, the memory manager, the object manager, and the I/O manager.
2. Loads non-critical drivers (those with start values > 0).
3. Transitions the display to **graphical mode** (via `win32k.sys` initialization).
4. Launches the **Session Manager Subsystem** — `smss.exe` — the first user-mode process.

### Phase 6: Session Manager (smss.exe)

The WinPE equivalent of `/sbin/init` on Linux.

1. Loads the remaining registry hives (`SOFTWARE`, `DEFAULT`, `SAM`, `SECURITY`, etc.).
2. Initializes the Win32 subsystem (`win32k.sys` kernel-mode, `csrss.exe` user-mode).
3. Starts `winlogon.exe`.
4. Starts the **Service Control Manager** (`services.exe`) and **LSASS** (`lsass.exe`).

### Phase 7: Winlogon → winpeshl.exe

1. `winlogon.exe` reads the registry value:
   ```
   HKLM\SYSTEM\Setup\CmdLine
   REG_SZ = winpeshl.exe
   ```
2. `winpeshl.exe` executes and does the following in order:
   - **Step A**: Launches `\windows\system32\WallpaperHost.exe` (sets the WinPE wallpaper).
   - **Step B**: Performs PnP device initialization (in a background thread).
   - **Step C**: Tries to launch `%SYSTEMDRIVE%\sources\setup.exe` — if it exists (used by Windows Setup).
   - **Step D**: Tries `x:\setup.exe` (alternative location).
   - **Step E**: Searches for `%SYSTEMROOT%\system32\winpeshl.ini`. If present and contains `[LaunchApps]`, executes those apps instead.
   - **Step F**: If none of the above match, falls back to:
     ```
     cmd.exe /k %SYSTEMROOT%\system32\startnet.cmd
     ```

### Phase 8: startnet.cmd → wpeinit.exe

Default `startnet.cmd` content:
```batch
wpeinit
```

`wpeinit.exe` performs:
- **Plug and Play device enumeration and driver installation** (storage, network, input, etc.).
- **Network stack initialization** (DHCP, DNS, TCP/IP).
- **Unattend.xml processing** if present (via `-unattend:` flag or `\unattend.xml` on the RAM drive).
- Logs all activity to `X:\windows\system32\wpeinit.log`.

### Phase 9: Ready State

When `wpeinit.exe` completes, the `cmd.exe` window remains open. The user (or an automated script) can now interact with the WinPE environment. The `X:` drive is backed by the FBWF (File Based Write Filter) overlay — all writes are stored in a RAM cache (default 32 MB, up to 512 MB depending on available RAM).

---

## 3. Customization Points

| Hook | Mechanism | Override |
|---|---|---|
| **Launch apps instead of cmd.exe** | Add `[LaunchApps]` section to `winpeshl.ini` | `%SYSTEMROOT%\system32\winpeshl.ini` |
| **Run custom scripts at boot** | Edit `startnet.cmd` (keep `wpeinit` call) | `%SYSTEMROOT%\system32\startnet.cmd` |
| **Unattended configuration** | Create `unattend.xml` with `<settings pass="windowsPE">` | `X:\unattend.xml` or `-unattend:` flag |
| **Run before wpeinit** | Registry `HKLM\SYSTEM\Setup\CmdLine` | Points to alternate shell |
| **Custom RAM disk size** | DISM: `Set-ScratchSpace` (32/64/128/256/512 MB) | `HKLM\SYSTEM\ControlSet001\Services\FBWF` → `WinPECacheThreshold` |

---

## 4. Key Architectural Details

### FBWF (File Based Write Filter)
- WinPE's `X:` drive is read-only at the WIM level.
- FBWF creates a **RAM overlay** that intercepts all writes.
- The overlay is discarded on reboot; nothing persists unless explicitly saved.

### BCD Ramdisk Syntax
```
device ramdisk=[source]\<path-to-wim>,{ramdiskoptions-guid}
osdevice ramdisk=[source]\<path-to-wim>,{ramdiskoptions-guid}
```
- `source` can be `boot` (ISO/USB root), `partition=C:`, or a GUID for a volume.
- `ramdiskoptions` GUID is a separate BCD object that holds the SDI path.

### Detection of WinPE Mode
- `winpe Yes` in BCD tells `winload.exe` to skip normal OS behaviours:
  - No crash dump configuration.
  - No pagefile creation.
  - No User Profile Service.
  - No Windows Search indexing.
  - Boot to `CmdLine` instead of logon UI.

---

## References

- [Microsoft Learn: Wpeinit and Startnet.cmd](https://learn.microsoft.com/en-us/windows-hardware/manufacture/desktop/wpeinit-and-startnetcmd-using-winpe-startup-scripts?view=windows-11)
- [Microsoft Learn: How Windows PE Works (archived)](https://learn.microsoft.com/en-us/previous-versions/windows/it-pro/windows-vista/cc721977(v=ws.10))
- [OSDeploy.com: WinPE Boot Process](https://www.osdeploy.com/inside-winpe/boot-environments/winpe-preinstallation-environment-boot.wim.md)
- [Sizing PE's RAM Drive (RW Internals)](https://rw.internals.io/post/sizing-pes-ram-drive/)
- [Windows PE Startup Sequence Explained (Slightly Overcomplicated)](https://slightlyovercomplicated.com/2016/11/07/windows-pe-startup-sequence-explained/)

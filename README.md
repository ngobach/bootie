# Bootie — Super rescue disk toolkit

<p>
  <a href="https://ngobach.github.io/bootie/"><img src="https://img.shields.io/badge/Online_Demo-v86-512BD4?logo=javascript" alt="Online Demo"></a>
  <img src="https://img.shields.io/badge/Go-1.24+-00ADD8?logo=go" alt="Go">
  <img src="https://img.shields.io/badge/C11-freestanding-00599C?logo=c" alt="C11">
  <img src="https://img.shields.io/badge/platform-macOS_|_Linux_|_Windows-lightgrey" alt="Platform">
  <img src="https://img.shields.io/badge/arch-x86__64_|_ARM64-important" alt="Architecture">
  <img src="https://img.shields.io/badge/license-MIT-green" alt="License">
</p>

A portable USB/CD boot/rescue disk image builder (GPT + EFI + data partition) with a suite of bare-metal external programs for GRUB4DOS/GRUB4EFI.

Two sub-projects:

- **`bootie-go/`** — Go CLI that partitions, formats, installs boot sectors, and writes data. Cross‑platform (Go 1.24+, macOS/Linux/Windows).
- **`bootie-ext/`** — Bare-metal C graphics programs (menu, file browser, games, benchmark) compiled as GRUB4DOS/GRUB4EFI external modules. Freestanding `x86_64-elf-gcc`.

---

## bootie-go — CLI toolkit

### Commands

| Command | Usage / Flags | Description |
|---------|---------------|-------------|
| **`list`** | *No flags* | Scans and lists all available target disks/devices. |
| **`init`** | `-t, --target <device>` *(Req)*<br>`-l, --layout <combined\|separate>` *(Default: separate)*<br>`-f, --fs <exfat\|fat32>` *(Default: exfat)* | Wipes target, creates GPT (EFI + data), formats, installs boot sectors, extracts embedded resources. |
| **`install`** | `-t, --target <device>` *(Req)* | Writes protective MBR + raw `grldr.mbr` loader to disk. |
| **`copy-efi`** | `-t, --target <mount>` *(Req)* | Copies embedded EFI/GRUB loaders to a mount point. |
| **`copy-data`** | `-t, --target <mount>` *(Req)* | Copies rescue data files to a mount point. |
| **`create-img`** | `-t, --target <file>` *(Req)*<br>`-s, --size <size>` *(Default: 200M)* | Creates an empty raw disk image file. |
| **`mount-img`** | `-t, --target <file>` *(Req)*<br>`-u, --unmount` | Mounts or detaches a raw disk image via `hdiutil`/`losetup`/`imdisk`. |
| **`run-qemu`** | `-t, --target <device>` *(Req)*<br>`-f, --firmware <path>`<br>`-v, --volume <folder>`<br>`-m, --memory <size>` *(Default: 2G)*<br>`-c, --cpus <count>` *(Default: 4)*<br>`-a, --arch <x86_64\|arm64>` *(Default: x86_64)* | Launches QEMU from the target device/image. |

### Build

```bash
make        # cross-compile bootie-go to build/ (linux, darwin, windows)
make linux  # linux only
make darwin # darwin only
```

### Example (bootable USB)

```bash
sudo ./build/bootie-linux init --target /dev/sdX
```

---

## bootie-ext — Bare-metal GRUB4DOS/GRUB4EFI programs

A collection of graphical external modules loaded by GRUB4DOS (BIOS) or GRUB4EFI (UEFI):

| Module | Description |
|--------|-------------|
| **menu** | Category-based boot menu with splash, file browser, log modals |
| **splash** | Animated "Bootie!" splash screen, auto-launches menu |
| **timer** | On-screen stopwatch |
| **file_browser** | Graphical file picker for booting ISOs/IMGs/EFIs |
| **tetris** | Classic tetris game |
| **snake** | Classic snake game |
| **breakout** | Breakout game |
| **nyan** | Nyan Cat animation demo |
| **screen_test** | Display test suite (static, fill bench, text bench) |
| **benchmark** | Bare-metal performance profiler (RDTSC, 12 tests) |

### Build (requires `x86_64-elf-gcc`)

```bash
make -C bootie-ext        # build all modules (BIOS + UEFI)
make -C bootie-ext bios   # BIOS only
make -C bootie-ext uefi   # UEFI only
```

Output: `bootie-ext/mod/bios/` and `bootie-ext/mod/uefi/`.

---

## Structure

```
├── bootie-go/        — Go CLI toolkit (commands, exfat/, resources/)
├── bootie-ext/       — Bare-metal C GRUB4DOS/GRUB4EFI externals
│   ├── src/          — Module source files
│   ├── include/      — Shared headers (bootie.h, bootie-gfx.h, …)
│   ├── libs/         — vendored libs (schrift, lodepng, stb)
│   └── ld/           — Linker scripts for raw binary output
├── resources/        — Firmware blobs (OVMF.fd, edk2-aarch64-code.fd)
├── runner/           — Test harness scripts & disk images
├── Makefile          — Top-level: builds bootie-go
└── README.md
```

## Safety

**Destructive operations.** `init` wipes the partition table and writes raw sectors. Verify the target device with `list`/`lsblk`/`diskutil`. Always use `sudo`. Backup data first.

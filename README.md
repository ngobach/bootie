# Bootie — Super rescue disk toolkit

CLI toolkit to build, initialize and install a portable boot/rescue disk image (GPT + EFI + data partition). Embeds EFI/GRUB assets and a raw seed image.

**Go 1.24+** • macOS/Linux/Windows

## Commands

| Command | Usage / Flags | Description |
|---------|---------------|-------------|
| **`list`** | *No flags* | Scans and lists all available target disks/devices on the host system. |
| **`init`** | `-t, --target <device>` *(Required)*<br>`-f, --fs <exfat\|fat32>` *(Default: exfat)*<br>`--no-data-part` *(Boolean)* | Wipes the target, partitions it with a GPT table (EFI + data), formats partitions, installs boot sectors, and extracts embedded resource files (skips formatting/copying data partition if `--no-data-part` is set). |
| **`install`** | `-t, --target <device>` *(Required)* | Installs the boot sectors (protective MBR + raw `grldr.mbr` loader) to the target disk. |
| **`copy-efi`** | `-t, --target <mount>` *(Required)* | Copies all embedded EFI/GRUB loader files to the specified directory/partition mount point. |
| **`copy-data`** | `-t, --target <mount>` *(Required)* | Copies all embedded system rescue/utility data files to the specified directory/partition mount point. |
| **`create-img`** | `-t, --target <file>` *(Required)*<br>`-s, --size <size>` *(Default: 200M)* | Creates an empty raw disk image file of the specified size (e.g., `200M`, `1G`). |
| **`mount-img`** | `-t, --target <file>` *(Required)*<br>`-u, --unmount` *(Boolean)* | Mounts/attaches a raw disk image file as a virtual disk device (uses `hdiutil` on macOS, `losetup` on Linux, `imdisk` on Windows). If `-u` is set, unmounts/detaches instead. |
| **`run-qemu`** | `-t, --target <device>` *(Required)*<br>`-f, --firmware <path>`<br>`-v, --volume <folder>`<br>`-m, --memory <size>` *(Default: 2G)*<br>`-c, --cpus <count>` *(Default: 4)*<br>`-a, --arch <x86_64\|arm64>` *(Default: x86_64)* | Launches QEMU virtual machine booting from the target device/image. Supports BIOS or UEFI firmware loading, architecture selection, CPU/memory scaling, and directory mounting. |

## Build

```bash
make   # cross-compile all platforms to build/
```

## Example (bootable USB)

```bash
sudo ./build/bootie-linux init --target /dev/sdX
```

## Structure

- `bootie-go/` — Go source (commands, platform disk managers, embedded `exfat/` filesystem writer, `resources/`)
- `resources/` — firmware binaries (`OVMF.fd`, `edk2-aarch64-code.fd`)
- `Makefile`

## Safety

**Destructive operations.** `init` wipes the partition table and writes raw sectors. Verify target device with `list`/`lsblk`/`diskutil`. Always use `sudo`. Backup data first.

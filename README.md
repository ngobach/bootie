# Bootie — Super rescue disk toolkit

CLI toolkit to build, initialize and install a portable boot/rescue disk image (GPT + EFI + data partition). Embeds EFI/GRUB assets and a raw seed image.

**Go 1.24+** • macOS/Linux/Windows

## Commands

| Command | Description |
|---------|-------------|
| `list` | List disks |
| `init --target <device> [--fs exfat\|fat32]` | Partition disk, create filesystems (default exfat), install boot sectors, and copy all embedded files |
| `install --target <device>` | Write raw boot seed sectors |
| `copy-efi --target <mount>` | Copy embedded EFI files to a mounted partition |
| `copy-data --target <mount>` | Copy data partition files to a mounted partition |
| `create-img --target <file> [--size 200M]` | Create an empty raw disk image |
| `run-qemu --target <device> [--firmware] [--arch x86_64\|arm64]` | Boot disk/image in QEMU |

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
- `resources/` — firmware binaries (`bios.bin`, `edk2-aarch64-code.fd`)
- `Makefile`

## Safety

**Destructive operations.** `init` wipes the partition table and writes raw sectors. Verify target device with `list`/`lsblk`/`diskutil`. Always use `sudo`. Backup data first.

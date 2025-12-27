# Bootie — Super rescue disk toolkit

Bootie is a small command-line toolkit to build, initialize and install a portable boot/rescue disk image (GPT + EFI + data partition), and to copy the included boot files to the correct partitions. It embeds a small set of EFI/GRUB assets and a raw "seed" (first sectors image) that the `install` command writes to a target disk.

This README describes what Bootie does, how to build it, how to test it safely (including with QEMU), and how to use the CLI. It also lists important safety notes and development pointers.

---

Table of Contents
- [Overview](#overview)
- [Quick start & prerequisites](#quick-start--prerequisites)
- [Build & run](#build--run)
- [Usage & commands](#usage--commands)
- [Example workflow (create a bootable usb)](#example-workflow-create-a-bootable-usb)
- [Testing with QEMU (`bootie.sh`)](#testing-with-qemu-bootiesh)
- [Project structure](#project-structure)
- [Embedded resources](#embedded-resources)
- [Security & safety warnings](#security--safety-warnings)
- [Troubleshooting & notes for maintainers](#troubleshooting--notes-for-maintainers)
- [Contributing](#contributing)
- [License](#license)

---

## Overview

Bootie helps you:
- Partition a target disk with a small EFI partition and a larger data partition.
- Write a pre-built "seed sectors" image (a small GPT + boot code region) to the beginning of the disk.
- Copy embedded EFI and data files into mounted partitions so the disk can boot a menu/GRUB or UEFI payloads.
- Provide a small helper script (`bootie.sh`) to create a small test image and run it in QEMU.

Bootie is written in Go and supports platform-specific disk discovery/interaction code for Linux, macOS (darwin) and Windows.

---

## Quick start & prerequisites

Minimum / recommended:
- Go 1.24 (module uses `go 1.24.5`).
- qemu (`qemu-system-x86_64`) if you want to run booted images in QEMU.
- On macOS: `diskutil` and `hdiutil` are used by helper scripts.
- Root / administrator privileges are required for raw disk operations (`init`, `install`).
- Always back up important data—these operations are destructive for the target device.

---

## Build & run

There is a simple `Makefile` included to build cross-platform binaries:

- Build all supported platform binaries (darwin, linux, windows):
```bash
make
# outputs: build/bootie-darwin, build/bootie-linux, build/bootie-windows.exe
```

- Clean build artifacts:
```bash
make clean
```

You can also run directly from source (useful during development):
```bash
cd bootie-go
go run . list
```

Or build a local binary:
```bash
cd bootie-go
go build -o ../build/bootie-local .
```

---

## Usage & commands

Run the (built or run-from-source) CLI named `bootie` (the `main` package defines the CLI).

Quick help:
```bash
bootie --help
# or, when running from source:
cd bootie-go && go run . --help
```

Commands:

- `list`  
  Detects disks on the host and prints a short list with identifier, label and size.
  Example:
  ```bash
  cd bootie-go && go run . list
  ```

- `init --target <device>`  
  Initializes (partitions and formats) the target disk with a GPT layout and creates:
    - an `EFI` FAT32 partition (~200MB)
    - a `Bootie` FAT32 data partition (rest of disk)  
  This is destructive for the chosen disk and will overwrite its partition table.
  Example:
  ```bash
  sudo ./build/bootie-linux init --target /dev/sdX
  ```

- `install --target <device>`  
  Verifies the target disk has a protective MBR/GPT signature and that partitions are positioned safely, then writes a raw "seed sectors" payload (embedded `gptdisk-64-sectors.raw`) to the beginning of the disk. Use with extreme caution: this writes raw sectors.
  Example:
  ```bash
  sudo ./build/bootie-linux install --target /dev/sdX
  ```

- `copy-efi --target <mountpoint>`  
  Copies embedded EFI files (`efi-part`) to the given mount point (e.g. your EFI partition mount).
  Example:
  ```bash
  # mount the EFI partition (example):
  sudo mount -t vfat /dev/sdX1 /mnt/efi
  ./build/bootie-linux copy-efi --target /mnt/efi
  ```

- `copy-data --target <mountpoint>`  
  Copies data files (`data-part`) to the target mount point.
  Example:
  ```bash
  sudo mount -t vfat /dev/sdX2 /mnt/bootie
  ./build/bootie-linux copy-data --target /mnt/bootie
  ```

Note: `copy-efi` and `copy-data` assume you supply a filesystem mount point. On macOS, mounting/unmounting is managed via `diskutil` (or done in Finder).

---

## Example workflow (create a bootable USB)

1. Identify target device:
   - Linux: `lsblk` / `sudo fdisk -l`
   - macOS: `diskutil list`
   - Bootie: `cd bootie-go && go run . list`

2. Initialize disk (creates partitions):
```bash
sudo ./build/bootie-linux init --target /dev/sdX
```

3. Install boot seed (writes the first sectors that contain the bootloader region):
```bash
sudo ./build/bootie-linux install --target /dev/sdX
```

4. Mount partitions and copy files:
```bash
sudo mount -t vfat /dev/sdX1 /mnt/efi
sudo mount -t vfat /dev/sdX2 /mnt/bootie
sudo ./build/bootie-linux copy-efi --target /mnt/efi
sudo ./build/bootie-linux copy-data --target /mnt/bootie
```

5. Unmount and test on a real machine or in QEMU.

---

## Testing with QEMU (`bootie.sh`)

The repository contains `bootie.sh` which provides utilities to run a disk image in QEMU and to create a small test image on macOS.

- Create a small test image (macOS only):
```bash
./bootie.sh create -t ./test-disk
```

- Run an image (regular x86):
```bash
./bootie.sh run -t test-disk
```

- Run in UEFI mode:
```bash
./bootie.sh run -u -t test-disk
```

`bootie.sh` checks for `qemu-system-x86_64`. The script will attempt to unmount physical disks on macOS before passing them to QEMU, and will require root for some operations.

---

## Project structure (high-level)

- `Makefile` — cross-build targets (darwin/linux/windows).
- `bootie.sh` — convenience script for qemu-based testing and image creation.
- `resources/` — top-level resources (e.g. `bios.bin` used by QEMU UEFI boot).
- `bootie-go/` — main Go implementation:
  - `main.go` — CLI and command implementations (`list`, `init`, `install`, `copy-efi`, `copy-data`).
  - `rawio.go` — raw disk read/write helpers and macOS unmount helpers.
  - `file_extractor.go` — utilities to extract embedded `fs` files to a target directory.
  - `scandisk_*.go` — platform-specific disk scanning (linux/darwin/windows).
  - `resources/` — embedded assets (`gptdisk-64-sectors.raw`, `efi-part/`, `data-part/`) via `go:embed`.
- `build/` — output directory for built binaries (created by `make`).

---

## Embedded resources

- `bootie-go/resources/gptdisk-64-sectors.raw`  
  The raw "seed" sectors written by `install`.

- `bootie-go/resources/efi-part/`  
  Contains sample EFI files (e.g. `EFI/BOOT/BOOTX64.EFI`) used for UEFI boot.

- `bootie-go/resources/data-part/`  
  Contains menu/GRUB config files (e.g. `menu.lst`) and helper payloads that live in the data partition.

These are embedded into the Go binary using `embed.FS` and extracted using `copy-efi` / `copy-data`.

---

## Security & safety warnings

- These operations are destructive. `init` modifies the partition table. `install` writes raw sectors. Always double check the target device path before running destructive commands.
- Use `list` and OS tools (`lsblk`, `diskutil`, `fdisk`) to confirm you have the right device.
- Run `init` and `install` with root privileges (e.g. `sudo`), but be careful.
- On macOS the tooling will try to unmount disks via `diskutil` before performing raw IO.

---

## Troubleshooting & notes for maintainers

- If you get permission errors, ensure you run as root for raw disk operations and that any partition you want to write files to is mounted with write permissions.
- If `list` reports no disks on Linux, check `/sys/block` and your environment's access to it. On macOS `diskutil` is used (plist parsing).
- The `install` flow performs several checks before writing:
  - It ensures the MBR signature is correct (`0x55 0xAA`) and that the MBR contains a protective partition (`0xEE`).
  - It checks GPT header signature (`"EFI PART"`).
  - It verifies that GPT partitions begin after the first 64 sectors.
- Note for maintainers: consider adding automated tests for the `install` and `init` logic using small disk images and QEMU. A review of the sector write loop and test coverage is recommended to ensure correctness and avoid accidental data loss.

---

## Contributing

Contributions are welcome. A few suggestions:
- Open issues for bugs and feature requests.
- Send PRs that include tests where feasible.
- Run `gofmt` / `go vet` and keep changes small and focused.

---

## License

No `LICENSE` file is included in this repository. If you'd like others to reuse this project under a specific license, add a `LICENSE` file (e.g., MIT, Apache-2.0) and note it here.

---

If you'd like, I can:
- Draft a concise `CONTRIBUTING.md` (PR template / local test workflow).
- Add example scripts that demonstrate a safe end-to-end test on QEMU.
- Add a small test suite that exercises `scandisk` and `file_extractor` on generated test images.

Tell me which of those you'd like me to work on next.
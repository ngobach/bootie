Super rescue disk
====

Prerequisites
----

1. A Linux machine to run bootlace.exe
1. GPT formatted rescue disk
1. 64MB gap after partition table for grldr.mbr
1. FAT32 formatted EFI partition
1. exFat formatted data partition

Installation
----

1. Install stage0 & stage1 bootloader with `sudo ./run.sh /dev/sdX` (under Linux / DOS).
1. Copy `./efi-part/*` to EFI partition.
1. Copy `./data-part/*` to data partition and download required files to this partition.

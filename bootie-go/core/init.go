package core

import (
	"fmt"
	"runtime"
	"strings"

	diskfs "github.com/diskfs/go-diskfs"
	diskfsFile "github.com/diskfs/go-diskfs/backend/file"
	diskfsDisk "github.com/diskfs/go-diskfs/disk"
	"github.com/diskfs/go-diskfs/filesystem"
	"github.com/diskfs/go-diskfs/partition/gpt"
	humanize "github.com/dustin/go-humanize"

	log "github.com/charmbracelet/log"
	"ngobach.com/bootie-go/exfat"
	"ngobach.com/bootie-go/resources"
)

func InitializeDisk(target, layout, fsType string, noDataCopy bool) error {
	if layout != "combined" && layout != "separate" {
		return fmt.Errorf("unsupported layout %q (must be combined or separate)", layout)
	}

	log.Infof("Host OS: %s", runtime.GOOS)
	log.Infof("Target to be initialized: %s", target)
	log.Infof("Layout: %s", layout)

	rawDisk, err := diskfsFile.OpenFromPath(target, false)

	if err != nil {
		return fmt.Errorf("failed to open disk: %w", err)
	}

	defer rawDisk.Close()

	disk, err := diskfs.OpenBackend(rawDisk)

	if err != nil {
		return fmt.Errorf("failed to open disk: %w", err)
	}

	log.Infof("Disk size: %s", humanize.IBytes(uint64(disk.Size)))

	minSizeSeparate := uint64(250 * 1024 * 1024) // 250 MiB
	minSizeCombined := uint64(4 * 1024 * 1024)   // 4 MiB
	switch layout {
	case "separate":
		if uint64(disk.Size) < minSizeSeparate {
			return fmt.Errorf("disk too small for %q layout: need at least %s, got %s",
				layout, humanize.IBytes(minSizeSeparate), humanize.IBytes(uint64(disk.Size)))
		}
	case "combined":
		if uint64(disk.Size) < minSizeCombined {
			return fmt.Errorf("disk too small for %q layout: need at least %s, got %s",
				layout, humanize.IBytes(minSizeCombined), humanize.IBytes(uint64(disk.Size)))
		}
	}

	var partitions []*gpt.Partition

	switch layout {
	case "combined":
		partitions = []*gpt.Partition{
			{
				Start: 1 * 1024 * 1024 / 512,
				End:   uint64(disk.Size)/512 - 1*1024*1024/512,
				Type:  gpt.MicrosoftBasicData,
				Name:  "Bootie",
			},
		}
	case "separate":
		partitions = []*gpt.Partition{
			{
				Start: 1 * 1024 * 1024 / 512,
				Size:  200 * 1024 * 1024,
				Type:  gpt.EFISystemPartition,
				Name:  "EFI",
			},
			{
				Start: 201 * 1024 * 1024 / 512,
				End:   uint64(disk.Size)/512 - 1*1024*1024/512,
				Type:  gpt.MicrosoftBasicData,
				Name:  "Bootie",
			},
		}
	}

	if err = disk.Partition(&gpt.Table{
		LogicalSectorSize:  512,
		PhysicalSectorSize: 512,
		ProtectiveMBR:      true,
		Partitions:         partitions,
	}); err != nil {
		if runtime.GOOS == "darwin" && strings.Contains(err.Error(), "inappropriate ioctl for device") {
			log.Warn("Ignoring macOS partition table re-read error (inappropriate ioctl for device)")
		} else {
			return fmt.Errorf("failed to partition disk: %w", err)
		}
	}

	w, err := disk.Backend.Writable()
	if err != nil {
		return fmt.Errorf("failed to get writable backend: %w", err)
	}
	if _, err := w.WriteAt([]byte{0x00, 0x02, 0x00}, 447); err != nil {
		return fmt.Errorf("failed to write MBR CHS start: %w", err)
	}
	if _, err := w.WriteAt([]byte{0xFE, 0xFF, 0xFF}, 451); err != nil {
		return fmt.Errorf("failed to write MBR CHS end: %w", err)
	}

	log.Info("Installing boot sectors...")
	if err := InstallTo(target); err != nil {
		return fmt.Errorf("failed to install boot sectors: %w", err)
	}

	switch layout {
	case "combined":
		log.Info("Creating Bootie partition (FAT32)")
		fsSpec := diskfsDisk.FilesystemSpec{
			Partition:   1,
			FSType:      filesystem.TypeFat32,
			VolumeLabel: "Bootie",
		}

		fs, err := disk.CreateFilesystem(fsSpec)
		if err != nil {
			return fmt.Errorf("failed to create Bootie partition: %w", err)
		}

		if err := CopyToFilesystem(resources.EfiFiles, "efi-part", fs); err != nil {
			fs.Close()
			return fmt.Errorf("failed to copy EFI files: %w", err)
		}

		if !noDataCopy {
			if err := CopyToFilesystem(resources.DataFiles, "data-part", fs); err != nil {
				fs.Close()
				return fmt.Errorf("failed to copy data files: %w", err)
			}
		}

		if err := fs.Close(); err != nil {
			return fmt.Errorf("failed to close FAT32: %w", err)
		}

	case "separate":
		{
			log.Info("Creating EFI partition")
			fsSpec := diskfsDisk.FilesystemSpec{
				Partition:   1,
				FSType:      filesystem.TypeFat32,
				VolumeLabel: "EFI",
			}

			efiFs, err := disk.CreateFilesystem(fsSpec)
			if err != nil {
				return fmt.Errorf("failed to create EFI partition: %w", err)
			}

			if err := CopyToFilesystem(resources.EfiFiles, "efi-part", efiFs); err != nil {
				efiFs.Close()
				return fmt.Errorf("failed to copy EFI files: %w", err)
			}

			if err := efiFs.Close(); err != nil {
				return fmt.Errorf("failed to close EFI filesystem: %w", err)
			}
		}

		{
			log.Infof("Creating Bootie partition (%s)", strings.ToUpper(fsType))
			partitions := disk.Table.GetPartitions()
			part := partitions[1]
			start := part.GetStart()
			size := part.GetSize()

			switch fsType {
			case "exfat":
				w, err := disk.Backend.Writable()
				if err != nil {
					return fmt.Errorf("failed to get writable backend: %w", err)
				}
				if err := exfat.CreateExfat(w, start, size, "Bootie"); err != nil {
					return fmt.Errorf("failed to create Bootie exFAT: %w", err)
				}

				ef, err := exfat.NewExfatFromBackend(disk.Backend, start)
				if err != nil {
					return fmt.Errorf("failed to mount exFAT: %w", err)
				}

				if !noDataCopy {
					if err := CopyToFilesystem(resources.DataFiles, "data-part", ef); err != nil {
						ef.Close()
						return fmt.Errorf("failed to copy data files: %w", err)
					}
				}

				if err := ef.Close(); err != nil {
					return fmt.Errorf("failed to close exFAT: %w", err)
				}
			default:
				fsSpec := diskfsDisk.FilesystemSpec{
					Partition:   2,
					FSType:      filesystem.TypeFat32,
					VolumeLabel: "Bootie",
				}

				dataFs, err := disk.CreateFilesystem(fsSpec)
				if err != nil {
					return fmt.Errorf("failed to create Bootie partition: %w", err)
				}

				if !noDataCopy {
					if err := CopyToFilesystem(resources.DataFiles, "data-part", dataFs); err != nil {
						dataFs.Close()
						return fmt.Errorf("failed to copy data files: %w", err)
					}
				}

				if err := dataFs.Close(); err != nil {
					return fmt.Errorf("failed to close FAT32: %w", err)
				}
			}
		}
	}

	log.Default().Logf(SuccessLevel, "Successfully initialized to %s", target)
	return nil
}

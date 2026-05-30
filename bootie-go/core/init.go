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

func InitializeDisk(target, fsType string, noDataPart bool) error {
	log.Infof("Host OS: %s", runtime.GOOS)
	log.Infof("Target to be initialized: %s", target)

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

	partitions := []*gpt.Partition{
		{
			Start: 1 * 1024 * 1024 / 512,
			Size:  200 * 1024 * 1024,
			Type:  gpt.EFISystemPartition,
			Name:  "EFI",
		},
	}

	if !noDataPart {
		partitions = append(partitions, &gpt.Partition{
			Start: 201 * 1024 * 1024 / 512,
			End:   uint64(disk.Size)/512 - 1*1024*1024/512,
			Type:  gpt.MicrosoftBasicData,
			Name:  "Bootie",
		})
	}

	if err = disk.Partition(&gpt.Table{
		LogicalSectorSize:  512,
		PhysicalSectorSize: 512,
		ProtectiveMBR:      true,
		Partitions:         partitions,
	}); err != nil {
		// On macOS, go-diskfs fails with ENOTTY (inappropriate ioctl for device)
		// when re-reading the partition table on a real block device, because
		// it uses the Linux-specific BLKRRPART ioctl under the hood.
		if runtime.GOOS == "darwin" && strings.Contains(err.Error(), "inappropriate ioctl for device") {
			log.Warn("Ignoring macOS partition table re-read error (inappropriate ioctl for device)")
		} else {
			return fmt.Errorf("failed to partition disk: %w", err)
		}
	}

	// Fix protective MBR CHS values (go-diskfs leaves them zeroed)
	w, err := disk.Backend.Writable()
	if err != nil {
		return fmt.Errorf("failed to get writable backend: %w", err)
	}
	// CHS start: 0x000200 (head=0, sector=2, cylinder=0)
	if _, err := w.WriteAt([]byte{0x00, 0x02, 0x00}, 447); err != nil {
		return fmt.Errorf("failed to write MBR CHS start: %w", err)
	}
	// CHS end: 0xFEFFFF (max values)
	if _, err := w.WriteAt([]byte{0xFE, 0xFF, 0xFF}, 451); err != nil {
		return fmt.Errorf("failed to write MBR CHS end: %w", err)
	}

	log.Info("Installing boot sectors...")
	if err := InstallTo(target); err != nil {
		return fmt.Errorf("failed to install boot sectors: %w", err)
	}

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

	if !noDataPart {
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

			if err := CopyToFilesystem(resources.DataFiles, "data-part", ef); err != nil {
				ef.Close()
				return fmt.Errorf("failed to copy data files: %w", err)
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

			if err := CopyToFilesystem(resources.DataFiles, "data-part", dataFs); err != nil {
				dataFs.Close()
				return fmt.Errorf("failed to copy data files: %w", err)
			}

			if err := dataFs.Close(); err != nil {
				return fmt.Errorf("failed to close FAT32: %w", err)
			}
		}
	}

	log.Default().Logf(SuccessLevel, "Successfully initialized to %s", target)
	return nil
}

package main

import (
	"context"
	"fmt"
	"runtime"
	"strings"

	diskfs "github.com/diskfs/go-diskfs"
	diskfsFile "github.com/diskfs/go-diskfs/backend/file"
	diskfsDisk "github.com/diskfs/go-diskfs/disk"
	"github.com/diskfs/go-diskfs/filesystem"
	"github.com/diskfs/go-diskfs/partition/gpt"
	humanize "github.com/dustin/go-humanize"
	"github.com/urfave/cli/v3"

	log "github.com/charmbracelet/log"
	"ngobach.com/bootie-go/exfat"
	"ngobach.com/bootie-go/resources"
)

func initializeDisk(target, fsType string) error {
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

	if err = disk.Partition(&gpt.Table{
		LogicalSectorSize:  512,
		PhysicalSectorSize: 512,
		ProtectiveMBR:      true,
		Partitions: []*gpt.Partition{
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
		},
	}); err != nil {
		return fmt.Errorf("failed to partition disk: %w", err)
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
	if _, err := w.WriteAt([]byte{0xFE, 0xFF, 0xFF}, 452); err != nil {
		return fmt.Errorf("failed to write MBR CHS end: %w", err)
	}

	log.Info("Installing boot sectors...")
	if err := installTo(target); err != nil {
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

		if err := copyToFilesystem(resources.EfiFiles, "efi-part", efiFs); err != nil {
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

			if err := copyToFilesystem(resources.DataFiles, "data-part", ef); err != nil {
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

			if err := copyToFilesystem(resources.DataFiles, "data-part", dataFs); err != nil {
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

func initCommand() *cli.Command {
	return &cli.Command{
		Name:  "init",
		Usage: "Partition disk and create filesystems with embedded data",
		Flags: []cli.Flag{
			&cli.StringFlag{
				Name:     "target",
				Aliases:  []string{"t"},
				Required: true,
			},
			&cli.StringFlag{
				Name:    "fs",
				Aliases: []string{"f"},
				Value:   "exfat",
				Usage:   "Filesystem for data partition: exfat (default) or fat32",
			},
		},
		Action: func(_ context.Context, c *cli.Command) error {
			target := c.String("target")
			fsType := c.String("fs")
			if fsType != "fat32" && fsType != "exfat" {
				return fmt.Errorf("unsupported filesystem %q (must be fat32 or exfat)", fsType)
			}
			return initializeDisk(target, fsType)
		},
	}
}

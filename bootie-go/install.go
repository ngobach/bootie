package main

import (
	"context"
	"fmt"
	"runtime"

	diskfs "github.com/diskfs/go-diskfs"
	diskfsFile "github.com/diskfs/go-diskfs/backend/file"
	"github.com/diskfs/go-diskfs/partition/gpt"
	"github.com/urfave/cli/v3"
	"ngobach.com/bootie-go/resources"

	log "github.com/charmbracelet/log"
)

func verifyDisk(target string) error {
	rawDisk, err := diskfsFile.OpenFromPath(target, false)
	if err != nil {
		return fmt.Errorf("failed to open disk: %w", err)
	}
	defer rawDisk.Close()

	disk, err := diskfs.OpenBackend(rawDisk)
	if err != nil {
		return fmt.Errorf("failed to read disk: %w", err)
	}

	if disk.Table == nil {
		return fmt.Errorf("no partition table found on disk")
	}

	gptTable, ok := disk.Table.(*gpt.Table)
	if !ok {
		return fmt.Errorf("disk does not have a GPT partition table")
	}

	for _, part := range gptTable.Partitions {
		log.Debugf("Partition: %s (LBA %d, size %d)", part.Name, part.Start, part.GetSize())
		if part.Start < 64 {
			return fmt.Errorf("partition %q starts before 64 sectors (LBA: %d)", part.Name, part.Start)
		}
	}

	return nil
}

func installTo(target string) error {
	const sectorSize = 512
	const grldrMBRStartSector = 34

	if len(resources.GrldrMBR)%sectorSize != 0 {
		return fmt.Errorf("grldr.mbr resource is not a multiple of %d bytes", sectorSize)
	}

	if len(resources.GrldrMBR) != 16*sectorSize {
		return fmt.Errorf("grldr.mbr resource must be exactly 16 sectors")
	}

	log.Infof("Host OS: %s", runtime.GOOS)
	log.Infof("Target to be installed: %s", target)
	log.Info("Verifying disk before installing...")

	if err := verifyDisk(target); err != nil {
		return fmt.Errorf("disk verification failed: %w", err)
	}

	if err := DefaultDiskManager.LockDisk(target); err != nil {
		log.Warnf("Failed to lock/unmount disk: %v", err)
	}

	back, err := diskfsFile.OpenFromPath(target, false)
	if err != nil {
		return fmt.Errorf("failed to open disk: %w", err)
	}
	defer back.Close()

	w, err := back.Writable()
	if err != nil {
		return fmt.Errorf("failed to get writable backend: %w", err)
	}

	log.Info("Installing...")

	mbr := make([]byte, sectorSize)
	copy(mbr, resources.MBR)

	originalMBR := make([]byte, sectorSize)
	if _, err := back.ReadAt(originalMBR, 0); err != nil {
		return fmt.Errorf("failed to read original MBR: %w", err)
	}
	copy(mbr[440:512], originalMBR[440:512])

	mbr[510] = 0x55
	mbr[511] = 0xAA

	if _, err := w.WriteAt(mbr, 0); err != nil {
		return fmt.Errorf("failed to write MBR: %w", err)
	}

	for i := range len(resources.GrldrMBR) / sectorSize {
		offset := i * sectorSize
		sector := grldrMBRStartSector + i
		if _, err := w.WriteAt(resources.GrldrMBR[offset:offset+sectorSize], int64(sector*sectorSize)); err != nil {
			return fmt.Errorf("failed to write sector %d: %w", sector, err)
		}
	}
	log.Default().Logf(SuccessLevel, "Successfully installed to %s", target)
	return nil
}

func installCommand() *cli.Command {
	return &cli.Command{
		Name:  "install",
		Usage: "Install bootie to given target",
		Flags: []cli.Flag{
			&cli.StringFlag{
				Name:     "target",
				Aliases:  []string{"t"},
				Required: true,
			},
		},
		Action: func(_ context.Context, c *cli.Command) error {
			target := c.String("target")
			return installTo(target)
		},
	}
}

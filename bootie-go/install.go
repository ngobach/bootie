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
	seedSizeInBytes := len(resources.SeedSectors)
	if seedSizeInBytes%512 != 0 {
		return fmt.Errorf("seedSectors is not a multiple of 512")
	}
	numSectors := seedSizeInBytes / 512

	log.Infof("Host OS: %s", runtime.GOOS)
	log.Infof("Target to be installed: %s", target)
	log.Info("Verifying disk before installing...")

	if err := verifyDisk(target); err != nil {
		return fmt.Errorf("disk verification failed: %w", err)
	}

	rawIo, err := OpenRawIo(target)
	if err != nil {
		return fmt.Errorf("failed to open raw io: %w", err)
	}
	defer rawIo.Close()

	log.Info("Installing...")

	buffer := make([]byte, seedSizeInBytes)
	copy(buffer, resources.SeedSectors)
	originalMBR, err := rawIo.ReadSector(0)
	if err != nil {
		return fmt.Errorf("failed to read original MBR: %w", err)
	}
	copy(buffer[446:512], originalMBR[446:512])
	for i := 1; i <= 33; i++ {
		originalSector, err := rawIo.ReadSector(int64(i))
		if err != nil {
			return fmt.Errorf("failed to read original sector number %d: %w", i, err)
		}
		copy(buffer[i*512:], originalSector)
	}
	for i := range numSectors {
		if err := rawIo.WriteSector(int64(i), buffer[i*512:(i+1)*512]); err != nil {
			return fmt.Errorf("failed to write sector %d: %w", i, err)
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

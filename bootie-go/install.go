package main

import (
	"context"
	"fmt"
	"runtime"

	"github.com/urfave/cli/v3"
	"ngobach.com/bootie-go/resources"

	log "github.com/charmbracelet/log"
)

func verifyProtectiveMbr(rawIo *RawIo) error {
	originalMBR, err := rawIo.ReadSector(0)

	if err != nil {
		return fmt.Errorf("failed to read original MBR: %w", err)
	}

	if originalMBR[0x1fe] != 0x55 || originalMBR[0x1ff] != 0xAA {
		return fmt.Errorf("original MBR is not valid")
	}

	if originalMBR[0x1be+0x4] != 0xEE {
		return fmt.Errorf("first partition is not protective MBR partition")
	}

	return nil
}

func verifyGptHeader(rawIo *RawIo) error {
	buffer, err := rawIo.ReadSector(1)

	if err != nil {
		return fmt.Errorf("failed to read original MBR: %w", err)
	}

	signature := buffer[:0x8]

	if string(signature) != "EFI PART" {
		return fmt.Errorf("signature is not EFI PART")
	}

	return nil
}

func verifyGptPartitionPosition(rawIo *RawIo) error {
	for i := 2; i < 34; i++ {
		buffer, err := rawIo.ReadSector(int64(i))

		if err != nil {
			return fmt.Errorf("failed to read sector: %w", err)
		}

		for offset := 0; offset < 512; offset += 128 {
			partitionRawData := buffer[offset : offset+128]
			part, err := parseGptPart(partitionRawData)

			if err != nil {
				return fmt.Errorf("failed to parse partition: %w", err)
			}

			if part.isEmpty() {
				continue
			}

			printGptPart(part)
			log.Debug("---")

			if part.firstLBA < 64 {
				return fmt.Errorf("partition starts before 64 sectors (LBA: %d)", part.firstLBA)
			}
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

	rawIo, err := OpenRawIo(target)
	if err != nil {
		return fmt.Errorf("failed to open raw io: %w", err)
	}
	defer rawIo.Close()

	if err := verifyProtectiveMbr(rawIo); err != nil {
		return fmt.Errorf("failed to verify protective MBR: %w", err)
	}

	if err := verifyGptHeader(rawIo); err != nil {
		return fmt.Errorf("failed to verify GPT header: %w", err)
	}

	if err := verifyGptPartitionPosition(rawIo); err != nil {
		return fmt.Errorf("failed to verify GPT partition position: %w", err)
	}

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

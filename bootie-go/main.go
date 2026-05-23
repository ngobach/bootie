package main

import (
	"context"
	"fmt"
	"os"
	"runtime"

	humanize "github.com/dustin/go-humanize"
	"github.com/urfave/cli/v3"
	"ngobach.com/bootie-go/resources"
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
	// All GPT partitions must start after the first 64 sectors
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
			fmt.Println("---")

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

	fmt.Printf("Host OS: %s\n", runtime.GOOS)
	fmt.Printf("Target to be installed: %s\n", target)
	fmt.Printf("Verifying disk before installing...\n")

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

	fmt.Println("Installing...")

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
	fmt.Printf("Successfully installed to %s\n", target)
	return nil
}

func main() {
	cmd := &cli.Command{
		Name:  "bootie",
		Usage: "Bootie commandline tool",
		Commands: []*cli.Command{
			initCommand(),
			{
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
			},
			{
				Name:  "list",
				Usage: "List available targets in current system",
				Action: func(c context.Context, cmd *cli.Command) error {
					fmt.Printf("Host OS: %s\n", runtime.GOOS)
					result, err := scanDisk()

					if err != nil {
						return fmt.Errorf("failed to scan disk: %w", err)
					}

					fmt.Printf("Found %d disk(s):\n", len(result))

					for _, disk := range result {
						fmt.Printf("- Path: %s, label: %s, size: %s\n", disk.identifier, disk.label, humanize.IBytes(uint64(disk.size)))
					}

					return nil
				},
			},
			{
				Name:  "copy-efi",
				Usage: "Copy EFI files to EFI partition",
				Flags: []cli.Flag{
					&cli.StringFlag{
						Name:     "target",
						Aliases:  []string{"t"},
						Required: true,
					},
				},
				Action: func(c context.Context, cmd *cli.Command) error {
					target := cmd.String("target")
					return extractFiles(
						&resources.EfiFiles,
						"efi-part",
						target,
					)
				},
			},
			{
				Name:  "copy-data",
				Usage: "Copy data files to data partition",
				Flags: []cli.Flag{
					&cli.StringFlag{
						Name:     "target",
						Aliases:  []string{"t"},
						Required: true,
					},
				},
				Action: func(c context.Context, cmd *cli.Command) error {
					target := cmd.String("target")
					return extractFiles(
						&resources.DataFiles,
						"data-part",
						target,
					)
				},
			},
			{
				Name:  "create-img",
				Usage: "Create an empty raw disk image file",
				Flags: []cli.Flag{
					&cli.StringFlag{
						Name:     "target",
						Aliases:  []string{"t"},
						Required: true,
						Usage:    "Path to the output image file",
					},
					&cli.StringFlag{
						Name:    "size",
						Aliases: []string{"s"},
						Value:   "200M",
						Usage:   "Image size (e.g., 200M, 1G)",
					},
				},
				Action: func(_ context.Context, cmd *cli.Command) error {
					return createImg(cmd.String("target"), cmd.String("size"))
				},
			},
			createQemuCommand(),
		},
	}

	if err := cmd.Run(context.Background(), os.Args); err != nil {
		panic(err)
	}
}

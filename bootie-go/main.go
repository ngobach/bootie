package main

import (
	"context"
	"fmt"
	"log"
	"os"
	"runtime"

	"github.com/urfave/cli/v3"
	"ngobach.com/bootie-go/resources"
)

func cmd_list() error {
	fmt.Printf("Host OS: %s\n", runtime.GOOS)
	result, err := scanDisk()

	if err != nil {
		return fmt.Errorf("failed to scan disk: %w", err)
	}

	fmt.Printf("Found %d disk(s):\n", len(result))

	for _, disk := range result {
		fmt.Printf("- Path: %s, label: %s, size: %s\n", disk.identifier, disk.label, sizeToFriendly(disk.size))
	}

	return nil
}

func cmd_install(target string) error {
	seedSizeInBytes := len(resources.SeedSectors)
	if seedSizeInBytes%512 != 0 {
		return fmt.Errorf("seedSectors is not a multiple of 512")
	}
	numSectors := seedSizeInBytes / 512

	fmt.Printf("Host OS: %s\n", runtime.GOOS)
	fmt.Printf("Target to be installed: %s\n", target)
	fmt.Printf("WARNING: Make sure the first partition starts after the first %d bytes\n", seedSizeInBytes)
	fmt.Println("---")

	rawIo, err := OpenRawIo(target)
	if err != nil {
		return fmt.Errorf("failed to open raw io: %w", err)
	}
	defer rawIo.Close()

	buffer := make([]byte, seedSizeInBytes)
	copy(buffer, resources.SeedSectors)
	originalMBR, err := rawIo.ReadSector(0)
	if err != nil {
		return fmt.Errorf("failed to read original MBR: %w", err)
	}
	copy(buffer[0:446], originalMBR[0:446])
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
			{
				Name:  "install",
				Usage: "Install bootie to given target",
				Flags: []cli.Flag{
					&cli.StringFlag{
						Name:     "target",
						Required: true,
					},
				},
				Action: func(_ context.Context, c *cli.Command) error {
					target := c.String("target")
					return cmd_install(target)
				},
			},
			{
				Name:  "list",
				Usage: "List available targets in current system",
				Action: func(c context.Context, cmd *cli.Command) error {
					return cmd_list()
				},
			},
		},
	}

	if err := cmd.Run(context.Background(), os.Args); err != nil {
		log.Fatal(err)
	}
}

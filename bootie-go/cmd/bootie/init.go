package main

import (
	"context"
	"fmt"

	"github.com/urfave/cli/v3"
	"ngobach.com/bootie-go/core"
)

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
				Name:    "layout",
				Aliases: []string{"l"},
				Value:   "separate",
				Usage:   "Disk layout: combined (single FAT32) or separate (EFI + data partition)",
			},
			&cli.StringFlag{
				Name:    "fs",
				Aliases: []string{"f"},
				Value:   "exfat",
				Usage:   "Filesystem for data partition: exfat (default) or fat32 (only for separate layout)",
			},
			&cli.BoolFlag{
				Name:    "skip-data-copy",
				Usage:   "Skip copying user data files to the data partition",
			},
		},
		Action: func(_ context.Context, c *cli.Command) error {
			target := c.String("target")
			layout := c.String("layout")
			if layout != "combined" && layout != "separate" {
				return fmt.Errorf("unsupported layout %q (must be combined or separate)", layout)
			}
			fsType := c.String("fs")
			if layout == "combined" && fsType != "fat32" {
				fmt.Printf("Warning: --fs %q is ignored for combined layout (always FAT32)\n", fsType)
			}
			if fsType != "fat32" && fsType != "exfat" {
				return fmt.Errorf("unsupported filesystem %q (must be fat32 or exfat)", fsType)
			}
			noDataCopy := c.Bool("skip-data-copy")
			return core.InitializeDisk(target, layout, fsType, noDataCopy)
		},
	}
}

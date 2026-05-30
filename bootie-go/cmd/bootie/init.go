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
				Name:    "fs",
				Aliases: []string{"f"},
				Value:   "exfat",
				Usage:   "Filesystem for data partition: exfat (default) or fat32",
			},
			&cli.BoolFlag{
				Name:    "no-data-part",
				Usage:   "Skip formatting and copying files of the data partition",
			},
		},
		Action: func(_ context.Context, c *cli.Command) error {
			target := c.String("target")
			fsType := c.String("fs")
			if fsType != "fat32" && fsType != "exfat" {
				return fmt.Errorf("unsupported filesystem %q (must be fat32 or exfat)", fsType)
			}
			noDataPart := c.Bool("no-data-part")
			return core.InitializeDisk(target, fsType, noDataPart)
		},
	}
}

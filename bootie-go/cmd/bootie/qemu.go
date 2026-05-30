package main

import (
	"context"

	"github.com/urfave/cli/v3"
	"ngobach.com/bootie-go/core"
)

func runQemuCommand() *cli.Command {
	return &cli.Command{
		Name:  "run-qemu",
		Usage: "Run QEMU with a disk image or device",
		Flags: []cli.Flag{
			&cli.StringFlag{
				Name:     "target",
				Aliases:  []string{"t"},
				Usage:    "Target disk device or image file",
				Required: true,
			},
			&cli.StringFlag{
				Name:    "volume",
				Aliases: []string{"v"},
				Usage:   "Folder to mount as a virtual FAT disk drive",
			},
			&cli.StringFlag{
				Name:    "firmware",
				Aliases: []string{"f"},
				Usage:   "Path to UEFI firmware file",
			},
			&cli.StringFlag{
				Name:    "memory",
				Aliases: []string{"m"},
				Value:   "2G",
				Usage:   "Memory size (e.g., 2G, 1024M)",
			},
			&cli.IntFlag{
				Name:    "cpus",
				Aliases: []string{"c"},
				Value:   4,
				Usage:   "Number of CPUs",
			},
			&cli.StringFlag{
				Name:    "arch",
				Aliases: []string{"a"},
				Value:   "x86_64",
				Usage:   "Target architecture (x86_64 or arm64)",
			},
		},
		Action: func(c context.Context, cmd *cli.Command) error {
			runCmd := &core.QemuRunCommand{
				Target:   cmd.String("target"),
				Volume:   cmd.String("volume"),
				Firmware: cmd.String("firmware"),
				Memory:   cmd.String("memory"),
				Cpus:     int(cmd.Int("cpus")),
				Arch:     cmd.String("arch"),
				QemuArgs: cmd.Args().Slice(),
			}
			return runCmd.Execute()
		},
	}
}

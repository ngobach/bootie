package main

import (
	"context"

	"github.com/urfave/cli/v3"
	"ngobach.com/bootie-go/core"

	log "github.com/charmbracelet/log"
)

func mountImgCommand() *cli.Command {
	return &cli.Command{
		Name:  "mount-img",
		Usage: "Mount a raw disk image as a virtual disk device",
		Flags: []cli.Flag{
			&cli.StringFlag{
				Name:     "target",
				Aliases:  []string{"t"},
				Required: true,
				Usage:    "Path to the raw disk image file",
			},
			&cli.BoolFlag{
				Name:    "unmount",
				Aliases: []string{"u"},
				Usage:   "Unmount the virtual drive instead of mounting",
			},
		},
		Action: func(_ context.Context, cmd *cli.Command) error {
			target := cmd.String("target")
			if cmd.Bool("unmount") {
				if err := core.UnmountImage(target); err != nil {
					return err
				}
				log.Default().Logf(core.SuccessLevel, "Unmounted %s", target)
				return nil
			}
			device, err := core.MountImage(target)
			if err != nil {
				return err
			}
			log.Default().Logf(core.SuccessLevel, "Mounted at %s", device)
			return nil
		},
	}
}

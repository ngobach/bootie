package main

import (
	"context"

	"github.com/urfave/cli/v3"
	"ngobach.com/bootie-go/core"
)

func createImgCommand() *cli.Command {
	return &cli.Command{
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
			return core.CreateImg(cmd.String("target"), cmd.String("size"))
		},
	}
}

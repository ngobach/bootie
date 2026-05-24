package main

import (
	"context"

	"github.com/urfave/cli/v3"
	"ngobach.com/bootie-go/resources"
)

func copyEfiCommand() *cli.Command {
	return &cli.Command{
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
	}
}

func copyDataCommand() *cli.Command {
	return &cli.Command{
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
	}
}

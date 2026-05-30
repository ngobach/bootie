package main

import (
	"context"

	"github.com/urfave/cli/v3"
	"ngobach.com/bootie-go/core"
)

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
			return core.InstallTo(target)
		},
	}
}

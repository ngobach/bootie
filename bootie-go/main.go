package main

import (
	"context"
	"os"

	"github.com/urfave/cli/v3"
)

func main() {
	cmd := &cli.Command{
		Name:  "bootie",
		Usage: "Bootie commandline tool",
		Commands: []*cli.Command{
			initCommand(),
			installCommand(),
			listCommand(),
			copyEfiCommand(),
			copyDataCommand(),
			createImgCommand(),
			qemuCommand(),
		},
	}

	if err := cmd.Run(context.Background(), os.Args); err != nil {
		panic(err)
	}
}

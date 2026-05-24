package main

import (
	"context"
	"fmt"
	"os"

	humanize "github.com/dustin/go-humanize"
	"github.com/urfave/cli/v3"

	log "github.com/charmbracelet/log"
)

func createImg(target, sizeStr string) error {
	sizeBytes, err := parseSize(sizeStr)
	if err != nil {
		return fmt.Errorf("failed to parse size: %w", err)
	}

	file, err := os.Create(target)
	if err != nil {
		return fmt.Errorf("failed to create image file: %w", err)
	}
	defer file.Close()

	if err := file.Truncate(sizeBytes); err != nil {
		return fmt.Errorf("failed to set image size: %w", err)
	}

	log.Default().Logf(SuccessLevel, "Created raw disk image: %s (%s)", target, humanize.IBytes(uint64(sizeBytes)))
	return nil
}

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
			return createImg(cmd.String("target"), cmd.String("size"))
		},
	}
}

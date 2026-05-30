package main

import (
	"context"
	"fmt"
	"runtime"

	humanize "github.com/dustin/go-humanize"
	"github.com/urfave/cli/v3"

	log "github.com/charmbracelet/log"
	"ngobach.com/bootie-go/core"
)

func listCommand() *cli.Command {
	return &cli.Command{
		Name:  "list",
		Usage: "List available targets in current system",
		Action: func(c context.Context, cmd *cli.Command) error {
			log.Infof("Host OS: %s", runtime.GOOS)
			result, err := core.DefaultDiskManager.ScanDisks()

			if err != nil {
				return fmt.Errorf("failed to scan disk: %w", err)
			}

			log.Infof("Found %d disk(s):", len(result))

			for _, disk := range result {
				log.Infof("- Path: %s, label: %s, size: %s", disk.Identifier, disk.Label, humanize.IBytes(uint64(disk.Size)))
			}

			return nil
		},
	}
}

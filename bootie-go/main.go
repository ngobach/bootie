package main

import (
	"context"
	"fmt"
	"log"
	"os"
	"runtime"

	"github.com/urfave/cli/v3"
)

type diskEntry struct {
	identifier string
	label      string
	size       int64
}

func sizeToFriendly(size int64) string {
	if size < 1024 {
		return fmt.Sprintf("%d B", size)
	} else if size < 1024*1024 {
		return fmt.Sprintf("%.2f KB", float64(size)/1024)
	} else if size < 1024*1024*1024 {
		return fmt.Sprintf("%.2f MB", float64(size)/(1024*1024))
	} else {
		return fmt.Sprintf("%.2f GB", float64(size)/(1024*1024*1024))
	}
}

func cmd_install(target string) error {
	return fmt.Errorf("not implemented")
}

func cmd_list() error {
	fmt.Printf("Host OS: %s\n", runtime.GOOS)

	result, err := scanDisk()

	if err != nil {
		return fmt.Errorf("failed to scan disk: %w", err)
	}

	fmt.Printf("Found %d disk(s):\n", len(result))

	for _, disk := range result {
		fmt.Printf("- %s: %s (%s)\n", disk.identifier, disk.label, sizeToFriendly(disk.size))
	}

	return nil
}

func main() {
	cmd := &cli.Command{
		Name:  "bootie",
		Usage: "Bootie commandline tool",
		Commands: []*cli.Command{
			{
				Name:  "install",
				Usage: "Install bootie to given target",
				Arguments: []cli.Argument{
					&cli.StringArg{
						Name: "target",
						Min:  1,
						Max:  1,
					},
				},
				Action: func(c context.Context, cmd *cli.Command) error {
					target := cmd.Args().Get(1)
					return cmd_install(target)
				},
			},
			{
				Name:  "list",
				Usage: "List available targets in current system",
				Action: func(c context.Context, cmd *cli.Command) error {
					return cmd_list()
				},
			},
		},
	}

	if err := cmd.Run(context.Background(), os.Args); err != nil {
		log.Fatal(err)
	}
}

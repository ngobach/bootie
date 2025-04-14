package main

import (
	"flag"
	"fmt"
	"runtime"
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
	flag.Parse()

	if flag.NArg() == 0 {
		fmt.Printf("Usage:\n  bootie install <target>\n  bootie list\n")
		return
	}

	var err error

	switch flag.Arg(0) {
	case "install":
		err = cmd_install(flag.Arg(1))
	case "list":
		err = cmd_list()
	default:
		err = fmt.Errorf("unknown command: %s", flag.Arg(0))
	}

	if err != nil {
		panic(err)
	}
}

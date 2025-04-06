package main

import (
	"flag"
	"fmt"
	"os"
	"regexp"
	"runtime"
	"strconv"
	"strings"

	"ngobach.com/bootie/utils"
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

	result := []diskEntry{}

	if runtime.GOOS == "linux" {
		const basePath = "/sys/block"
		entries, err := os.ReadDir(basePath)

		if err != nil {
			return err
		}

		matcher := regexp.MustCompile("^[shv]d[a-z]$")

		for _, file := range entries {
			if matcher.MatchString(file.Name()) {
				vendor := strings.TrimSpace(utils.ReadFileAsString(fmt.Sprintf("%s/%s/device/vendor", basePath, file.Name())))
				model := strings.TrimSpace(utils.ReadFileAsString(fmt.Sprintf("%s/%s/device/model", basePath, file.Name())))
				sizeAsString := strings.TrimSpace(utils.ReadFileAsString(fmt.Sprintf("%s/%s/size", basePath, file.Name())))
				size := int64(0)

				if sizeAsString != "" {
					size, err = strconv.ParseInt(sizeAsString, 10, 64)

					if err == nil {
						size *= 512
					}
				}

				if vendor == "" {
					vendor = "Unknown"
				}

				if model == "" {
					model = "Unknown"
				}

				entry := diskEntry{
					identifier: fmt.Sprintf("%s/%s", basePath, file.Name()),
					label:      fmt.Sprintf("%s %s", vendor, model),
					size:       size,
				}

				result = append(result, entry)
			}
		}
	} else {
		return fmt.Errorf("unsupported OS")
	}

	fmt.Printf("Found %d disk(s):\n", len(result))

	for _, disk := range result {
		fmt.Printf("%s: %s (%s)\n", disk.identifier, disk.label, sizeToFriendly(disk.size))
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

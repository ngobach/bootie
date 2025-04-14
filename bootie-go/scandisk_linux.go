//go:build linux

package main

import (
	"fmt"
	"os"
	"regexp"
	"strconv"
	"strings"

	"ngobach.com/bootie/utils"
)

func scanDisk() ([]diskEntry, error) {
	result := []diskEntry{}

	const basePath = "/sys/block"
	entries, err := os.ReadDir(basePath)

	if err != nil {
		return nil, err
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

	return result, nil
}

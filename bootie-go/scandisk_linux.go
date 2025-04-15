//go:build linux

package main

import (
	"fmt"
	"os"
	"regexp"
	"strconv"
	"strings"
)

func readFileAsString(path string) string {
	file, err := os.Open(path)

	if err != nil {
		return ""
	}

	defer file.Close()

	bytes, err := os.ReadFile(path)

	if err != nil {
		return ""
	}

	return string(bytes)
}

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
			model := strings.TrimSpace(readFileAsString(fmt.Sprintf("%s/%s/device/model", basePath, file.Name())))
			sizeAsString := strings.TrimSpace(readFileAsString(fmt.Sprintf("%s/%s/size", basePath, file.Name())))
			size := int64(0)

			if sizeAsString != "" {
				size, err = strconv.ParseInt(sizeAsString, 10, 64)

				if err == nil {
					size *= 512
				}
			}

			if model == "" {
				model = "Unknown"
			}

			entry := diskEntry{
				identifier: fmt.Sprintf("%s", file.Name()),
				label:      fmt.Sprintf("%s", model),
				size:       size,
			}

			result = append(result, entry)
		}
	}

	return result, nil
}

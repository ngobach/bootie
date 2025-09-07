//go:build darwin

package main

import (
	"os/exec"
	"regexp"
	"strconv"
	"strings"

	"howett.net/plist"
)

type Output struct {
	WholeDisks []string
}

func entryFromDisk(disk string) diskEntry {
	cmd := exec.Command("diskutil", "info", disk)
	raw, _ := cmd.Output()
	lines := string(raw)
	entry := diskEntry{}
	entry.identifier = "/dev/" + disk

	// iterate over each line
	for line := range strings.SplitSeq(lines, "\n") {
		var k, v string
		if strings.Contains(line, ":") {
			k = strings.TrimSpace(line[:strings.Index(line, ":")])
			v = strings.TrimSpace(line[strings.Index(line, ":")+1:])

			switch k {
			case "Device / Media Name":
				entry.label = v
			case "Disk Size":
				pattern := regexp.MustCompile(`\((\d+) Bytes\)`)
				matches := pattern.FindStringSubmatch(v)
				entry.size, _ = strconv.ParseInt(matches[1], 10, 64)
			}
		}
	}

	return entry
}

func scanDisk() ([]diskEntry, error) {
	result := []diskEntry{}
	cmd := exec.Command("diskutil", "list", "-plist")
	raw, _ := cmd.Output()
	output := Output{}
	_, err := plist.Unmarshal(raw, &output)

	if err != nil {
		return nil, err
	}

	for _, disk := range output.WholeDisks {
		result = append(result, entryFromDisk(disk))
	}

	return result, nil
}

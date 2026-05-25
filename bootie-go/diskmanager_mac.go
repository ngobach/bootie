//go:build darwin

package main

import (
	"fmt"
	"os/exec"
	"regexp"
	"strconv"
	"strings"

	"howett.net/plist"

	log "github.com/charmbracelet/log"
)

type macDiskManager struct{}

func init() {
	DefaultDiskManager = &macDiskManager{}
}

type darwinOutput struct {
	WholeDisks []string
}

func (m *macDiskManager) entryFromDisk(disk string) diskEntry {
	cmd := exec.Command("diskutil", "info", disk)
	raw, _ := cmd.Output()
	lines := string(raw)
	entry := diskEntry{}
	entry.identifier = "/dev/" + disk

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

func (m *macDiskManager) ScanDisks() ([]diskEntry, error) {
	result := []diskEntry{}
	cmd := exec.Command("diskutil", "list", "-plist")
	raw, _ := cmd.Output()
	output := darwinOutput{}
	_, err := plist.Unmarshal(raw, &output)

	if err != nil {
		return nil, err
	}

	for _, disk := range output.WholeDisks {
		result = append(result, m.entryFromDisk(disk))
	}

	return result, nil
}

func (m *macDiskManager) LockDisk(path string) error {
	isDisk := strings.HasPrefix(path, "/dev/disk")
	if isDisk {
		diskId := strings.TrimPrefix(path, "/dev/")
		log.Infof("Attempting to unmount %s", path)
		cmd := exec.Command("diskutil", "umountDisk", "force", diskId)
		_, err := cmd.Output()
		if err != nil {
			return fmt.Errorf("failed to unmount disk: %w", err)
		}
	}

	return nil
}

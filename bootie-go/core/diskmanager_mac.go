//go:build darwin

package core

import (
	"fmt"
	"os"
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

type diskutilInfoPlist struct {
	DeviceIdentifier               string `plist:"DeviceIdentifier"`
	MediaName                      string `plist:"MediaName"`
	IORegistryEntryName            string `plist:"IORegistryEntryName"`
	TotalSize                      int64  `plist:"TotalSize"`
	Internal                       bool   `plist:"Internal"`
	Removable                      bool   `plist:"Removable"`
	RemovableMedia                 bool   `plist:"RemovableMedia"`
	RemovableMediaOrExternalDevice bool   `plist:"RemovableMediaOrExternalDevice"`
	Ejectable                      bool   `plist:"Ejectable"`
	VirtualOrPhysical              string `plist:"VirtualOrPhysical"`
}

func (m *macDiskManager) entryFromDisk(disk string) (DiskEntry, bool) {
	cmd := exec.Command("diskutil", "info", "-plist", disk)
	raw, err := cmd.Output()
	if err == nil {
		info := diskutilInfoPlist{}
		if _, err := plist.Unmarshal(raw, &info); err == nil {
			// Skip virtual / synthesized containers (e.g. APFS synthesized disk1, disk2)
			if info.VirtualOrPhysical == "Virtual" {
				return DiskEntry{}, false
			}
			isRemovable := info.Removable || info.RemovableMedia || info.RemovableMediaOrExternalDevice || info.Ejectable || !info.Internal
			label := info.MediaName
			if label == "" {
				label = info.IORegistryEntryName
			}
			if label == "" {
				label = "Storage Device"
			}
			return DiskEntry{
				Identifier:  "/dev/" + disk,
				Label:       label,
				Size:        info.TotalSize,
				IsRemovable: isRemovable,
				IsInternal:  info.Internal && !isRemovable,
			}, true
		}
	}

	// Fallback to text parsing
	cmdText := exec.Command("diskutil", "info", disk)
	rawText, _ := cmdText.Output()
	lines := string(rawText)
	entry := DiskEntry{Identifier: "/dev/" + disk}
	for line := range strings.SplitSeq(lines, "\n") {
		if strings.Contains(line, ":") {
			k := strings.TrimSpace(line[:strings.Index(line, ":")])
			v := strings.TrimSpace(line[strings.Index(line, ":")+1:])

			switch k {
			case "Device / Media Name":
				entry.Label = v
			case "VirtualOrPhysical":
				if v == "Virtual" {
					return DiskEntry{}, false
				}
			case "Device Location":
				if v == "External" {
					entry.IsRemovable = true
				} else if v == "Internal" {
					entry.IsInternal = true
				}
			case "Removable Media":
				if v == "Removable" {
					entry.IsRemovable = true
				}
			case "Disk Size":
				pattern := regexp.MustCompile(`\((\d+) Bytes\)`)
				matches := pattern.FindStringSubmatch(v)
				if len(matches) > 1 {
					entry.Size, _ = strconv.ParseInt(matches[1], 10, 64)
				}
			}
		}
	}

	return entry, true
}

func (m *macDiskManager) ScanDisks() ([]DiskEntry, error) {
	result := []DiskEntry{}
	cmd := exec.Command("diskutil", "list", "-plist")
	raw, _ := cmd.Output()
	output := darwinOutput{}
	_, err := plist.Unmarshal(raw, &output)

	if err != nil {
		return nil, err
	}

	for _, disk := range output.WholeDisks {
		if entry, ok := m.entryFromDisk(disk); ok {
			result = append(result, entry)
		}
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

func (m *macDiskManager) IsDevice(path string) (bool, error) {
	info, err := os.Stat(path)
	if err != nil {
		if os.IsNotExist(err) {
			return false, nil
		}
		return false, err
	}
	return (info.Mode() & os.ModeDevice) != 0, nil
}

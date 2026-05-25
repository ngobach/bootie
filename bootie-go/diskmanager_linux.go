//go:build linux

package main

import (
	"fmt"
	"os"
	"os/exec"
	"regexp"
	"strconv"
	"strings"

	"golang.org/x/sys/unix"

	log "github.com/charmbracelet/log"
)

type linuxDiskManager struct{}

func init() {
	DefaultDiskManager = &linuxDiskManager{}
}

func (l *linuxDiskManager) readFileAsString(path string) string {
	bytes, err := os.ReadFile(path)
	if err != nil {
		return ""
	}
	return string(bytes)
}

func (l *linuxDiskManager) ScanDisks() ([]diskEntry, error) {
	result := []diskEntry{}

	const basePath = "/sys/block"
	entries, err := os.ReadDir(basePath)
	if err != nil {
		return nil, err
	}

	matcher := regexp.MustCompile("^[shv]d[a-z]$")

	for _, file := range entries {
		if matcher.MatchString(file.Name()) {
			model := strings.TrimSpace(l.readFileAsString(fmt.Sprintf("%s/%s/device/model", basePath, file.Name())))
			sizeAsString := strings.TrimSpace(l.readFileAsString(fmt.Sprintf("%s/%s/size", basePath, file.Name())))
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
				identifier: fmt.Sprintf("/dev/%s", file.Name()),
				label:      model,
				size:       size,
			}

			result = append(result, entry)
		}
	}

	return result, nil
}

func (l *linuxDiskManager) LockDisk(path string) error {
	if !strings.HasPrefix(path, "/dev/") {
		return nil
	}

	log.Infof("Attempting to unmount %s", path)

	err := unix.Unmount(path, unix.MNT_FORCE)
	if err == nil {
		return nil
	}

	cmd := unixUnmountCmd(path)
	_, err = cmd.Output()
	if err != nil {
		return fmt.Errorf("failed to unmount %s: %w", path, err)
	}

	return nil
}

func unixUnmountCmd(path string) *exec.Cmd {
	return exec.Command("umount", "-l", path)
}

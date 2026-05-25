//go:build windows

package main

import (
	"fmt"
	"os/exec"
	"strings"

	"github.com/drtimf/wmi"

	log "github.com/charmbracelet/log"
)

type windowsDiskManager struct{}

func init() {
	DefaultDiskManager = &windowsDiskManager{}
}

func (w *windowsDiskManager) ScanDisks() ([]diskEntry, error) {
	var svc *wmi.Service
	var err error
	var result []diskEntry

	if svc, err = wmi.NewLocalService(wmi.RootCIMV2); err != nil {
		return nil, err
	}

	defer svc.Close()

	var w32Disk []wmi.Win32_DiskDrive

	if err = svc.Query("SELECT * FROM Win32_DiskDrive", &w32Disk); err != nil {
		return nil, err
	}

	for _, disk := range w32Disk {
		if !disk.MediaLoaded {
			continue
		}
		result = append(result, diskEntry{
			identifier: disk.DeviceID,
			label:      disk.Model,
			size:       int64(disk.Size),
		})
	}

	return result, nil
}

func (w *windowsDiskManager) LockDisk(path string) error {
	if !strings.HasPrefix(path, `\\.\PHYSICALDRIVE`) {
		return nil
	}

	diskNum := strings.TrimPrefix(path, `\\.\PHYSICALDRIVE`)
	log.Infof("Taking disk %s offline", diskNum)

	cmd := exec.Command("diskpart")
	cmd.Stdin = strings.NewReader(fmt.Sprintf("select disk %s\r\noffline disk\r\nexit\r\n", diskNum))
	output, err := cmd.CombinedOutput()
	if err != nil {
		return fmt.Errorf("failed to offline disk %s: %s: %w", diskNum, strings.TrimSpace(string(output)), err)
	}

	return nil
}

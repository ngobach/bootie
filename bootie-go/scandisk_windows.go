//go:build windows

package main

import (
	"strings"

	"github.com/0xrawsec/golang-win32/win32/kernel32"
	"github.com/drtimf/wmi"
)

func scanDisk2() ([]diskEntry, error) {
	dosDevices, err := kernel32.QueryDosDevice("")
	result := []diskEntry{}

	if err != nil {
		return nil, err
	}

	for _, device := range dosDevices {
		if strings.HasPrefix(device, "PhysicalDrive") {

		}
	}

	return result, nil
}

func scanDisk() ([]diskEntry, error) {
	var svc *wmi.Service
	var err error
	var result []diskEntry

	if svc, err = wmi.NewLocalService(wmi.RootCIMV2); err != nil {
		return nil, err
	}

	defer svc.Close()

	var w32Disk []wmi.Win32_DiskDrive

	if err = svc.Query("SELECT * FROM Win32_DiskDrive", &w32Disk); err != nil {
		panic(err)
	}

	for _, disk := range w32Disk {
		result = append(result, diskEntry{
			identifier: disk.DeviceID,
			label:      disk.Model,
			size:       int64(disk.Size),
		})
	}

	return result, nil
}

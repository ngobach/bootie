//go:build windows

package main

import (
	"fmt"
	"os/exec"

	"github.com/gocarina/gocsv"
)

type csvEntry struct {
	Caption    string
	Node       string
	DeviceID   string
	Model      string
	Partitions int32
	Size       int64
}

func scanDisk() ([]diskEntry, error) {
	csvEntries := []csvEntry{}
	result := []diskEntry{}
	cmd := exec.Command("wmic", "diskdrive", "list", "/format:csv")
	raw, err := cmd.Output()

	if err != nil {
		return nil, err
	}

	gocsv.UnmarshalBytes(raw, &csvEntries)

	for _, entry := range csvEntries {
		result = append(result, diskEntry{
			identifier: entry.DeviceID,
			label:      entry.Caption,
			size:       entry.Size,
		})
	}

	return nil, fmt.Errorf("not implemented")
}

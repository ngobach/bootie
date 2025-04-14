//go:build windows

package main

import "fmt"

func getDiskEntryByIdentifier(target string) (diskEntry, error) {
}

func scanDisk() ([]diskEntry, error) {
	return nil, fmt.Errorf("not implemented")
}

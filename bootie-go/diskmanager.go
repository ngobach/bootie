package main

type DiskManager interface {
	ScanDisks() ([]diskEntry, error)
	LockDisk(path string) error
}

type diskEntry struct {
	identifier string
	label      string
	size       int64
}

var DefaultDiskManager DiskManager

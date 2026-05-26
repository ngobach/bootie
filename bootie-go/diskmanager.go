package main

type DiskManager interface {
	ScanDisks() ([]diskEntry, error)
	LockDisk(path string) error
	IsDevice(path string) (bool, error)
}

type diskEntry struct {
	identifier string
	label      string
	size       int64
}

var DefaultDiskManager DiskManager

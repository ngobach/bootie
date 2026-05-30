package core

type DiskManager interface {
	ScanDisks() ([]DiskEntry, error)
	LockDisk(path string) error
	IsDevice(path string) (bool, error)
}

type DiskEntry struct {
	Identifier string
	Label      string
	Size       int64
}

var DefaultDiskManager DiskManager

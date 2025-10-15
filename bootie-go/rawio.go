package main

import (
	"fmt"
	"os"
	"os/exec"
	"runtime"
	"strings"
)

type RawIo struct {
	identifier string
	file       *os.File
}

func (rio *RawIo) ReadSector(sectorIndex int64) ([]byte, error) {
	_, err := rio.file.Seek(sectorIndex*512, 0)
	if err != nil {
		return nil, err
	}
	buffer := make([]byte, 512)
	_, err = rio.file.Read(buffer)
	if err != nil {
		return nil, err
	}
	return buffer, nil
}

func (rio *RawIo) WriteSector(sectorIndex int64, data []byte) error {
	_, err := rio.file.Seek(sectorIndex*512, 0)
	if err != nil {
		return err
	}
	_, err = rio.file.Write(data)
	if err != nil {
		return err
	}
	return nil
}

func (rio *RawIo) Close() error {
	return rio.file.Close()
}

func tryUnmountIfNeeded(path string) error {
	if runtime.GOOS == "darwin" {
		isDisk := strings.HasPrefix(path, "/dev/disk")
		if isDisk {
			diskId := strings.TrimPrefix(path, "/dev/")
			fmt.Printf("Attempting to unmount %s\n", path)
			cmd := exec.Command("diskutil", "umountDisk", "force", diskId)
			_, err := cmd.Output()
			if err != nil {
				return fmt.Errorf("failed to unmount disk: %w", err)
			}
		}
	}

	return nil
}

func OpenRawIo(path string) (*RawIo, error) {
	var f *os.File
	var err error

	if f, err = OpenRawDisk(path); err != nil {
		return nil, err
	}

	rawIo := RawIo{
		identifier: path,
		file:       f,
	}

	return &rawIo, nil
}

func OpenRawDisk(path string) (*os.File, error) {
	err := tryUnmountIfNeeded(path)

	if err != nil {
		return nil, err
	}

	f, err := os.OpenFile(path, os.O_RDWR, os.ModePerm)

	if err != nil {
		return nil, fmt.Errorf("failed to open file: %w", err)
	}

	return f, nil
}

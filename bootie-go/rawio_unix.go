//go:build darwin || linux

package main

import (
	"fmt"
	"os"
	"os/exec"
	"runtime"
	"strings"
)

type UnixRawIo struct {
	identifier string
	file       *os.File
}

func (mrio *UnixRawIo) ReadSector(sectorIndex int64) ([]byte, error) {
	_, err := mrio.file.Seek(sectorIndex*512, 0)
	if err != nil {
		return nil, err
	}
	buffer := make([]byte, 512)
	_, err = mrio.file.Read(buffer)
	if err != nil {
		return nil, err
	}
	return buffer, nil
}

func (mrio *UnixRawIo) WriteSector(sectorIndex int64, data []byte) error {
	_, err := mrio.file.Seek(sectorIndex*512, 0)
	if err != nil {
		return err
	}
	_, err = mrio.file.Write(data)
	if err != nil {
		return err
	}
	return nil
}

func (mrio *UnixRawIo) Close() error {
	return mrio.file.Close()
}

func OpenRawIo(path string) (RawIo, error) {
	if runtime.GOOS == "darwin" {
		isDisk := strings.HasPrefix(path, "/dev/disk")
		if isDisk {
			diskId := strings.TrimPrefix(path, "/dev/")
			fmt.Printf("Attempting to unmount %s\n", path)
			cmd := exec.Command("diskutil", "umountDisk", "force", diskId)
			_, err := cmd.Output()
			if err != nil {
				return nil, fmt.Errorf("failed to unmount disk: %w", err)
			}
		}
	}

	f, err := os.OpenFile(path, os.O_RDWR, os.ModePerm)
	if err != nil {
		return nil, fmt.Errorf("failed to open file: %w", err)
	}

	rawIo := UnixRawIo{
		identifier: path,
		file:       f,
	}

	return &rawIo, nil
}

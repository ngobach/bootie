//go:build windows

package main

import (
	"encoding/binary"
	"fmt"
	"strconv"
	"strings"

	"github.com/drtimf/wmi"
	"golang.org/x/sys/windows"

	log "github.com/charmbracelet/log"
)

const (
	fileDeviceFileSystem = 0x00000009
	fileDeviceVolume     = 0x00000056
	methodBuffered       = 0
	fileAnyAccess        = 0

	fsctlLockVolume              = fileDeviceFileSystem<<16 | fileAnyAccess<<14 | 6<<2 | methodBuffered
	fsctlDismountVolume          = fileDeviceFileSystem<<16 | fileAnyAccess<<14 | 8<<2 | methodBuffered
	ioctlVolumeGetDiskExtents    = fileDeviceVolume<<16 | fileAnyAccess<<14 | 0<<2 | methodBuffered
	volumeEnumerationBufferSize  = 1024
	volumeDiskExtentsBufferSize  = 64 * 1024
	volumeDiskExtentHeaderSize   = 8
	volumeDiskExtentSize         = 24
	volumeDiskExtentDiskNumStart = 0
)

type windowsDiskManager struct {
	lockedVolumes []windows.Handle
}

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
	diskNum, ok, err := physicalDriveNumber(path)
	if err != nil {
		return err
	}
	if !ok {
		return nil
	}

	log.Infof("Locking volumes on physical disk %d", diskNum)

	volumes, err := enumerateVolumes()
	if err != nil {
		return err
	}

	var locked []windows.Handle
	closeLocked := func() error {
		var firstErr error
		for _, handle := range locked {
			if err := windows.CloseHandle(handle); err != nil && firstErr == nil {
				firstErr = err
			}
		}
		return firstErr
	}

	for _, volume := range volumes {
		handle, err := openVolume(volume)
		if err != nil {
			return fmt.Errorf("failed to open volume %s: %w", volume, err)
		}

		belongs, err := volumeBelongsToDisk(handle, diskNum)
		if err != nil {
			windows.CloseHandle(handle)
			closeLocked()
			return fmt.Errorf("failed to inspect volume %s: %w", volume, err)
		}
		if !belongs {
			windows.CloseHandle(handle)
			continue
		}

		if err := deviceIoControl(handle, fsctlLockVolume, nil); err != nil {
			windows.CloseHandle(handle)
			closeLocked()
			return fmt.Errorf("failed to lock volume %s: %w", volume, err)
		}
		if err := deviceIoControl(handle, fsctlDismountVolume, nil); err != nil {
			windows.CloseHandle(handle)
			closeLocked()
			return fmt.Errorf("failed to dismount volume %s: %w", volume, err)
		}

		log.Infof("Locked and dismounted volume %s", volume)
		locked = append(locked, handle)
	}

	w.lockedVolumes = append(w.lockedVolumes, locked...)
	return nil
}

func (w *windowsDiskManager) IsDevice(path string) (bool, error) {
	_, ok, err := physicalDriveNumber(path)
	if err != nil {
		return false, err
	}
	return ok, nil
}

func physicalDriveNumber(path string) (uint32, bool, error) {
	const prefix = `\\.\PHYSICALDRIVE`
	if !strings.HasPrefix(path, prefix) {
		return 0, false, nil
	}

	raw := strings.TrimPrefix(path, prefix)
	if raw == "" {
		return 0, true, fmt.Errorf("missing physical drive number in %s", path)
	}
	for _, ch := range raw {
		if ch < '0' || ch > '9' {
			return 0, true, fmt.Errorf("invalid physical drive path %s", path)
		}
	}

	diskNum, err := strconv.ParseUint(raw, 10, 32)
	if err != nil {
		return 0, true, fmt.Errorf("invalid physical drive number %s: %w", raw, err)
	}

	return uint32(diskNum), true, nil
}

func enumerateVolumes() ([]string, error) {
	buffer := make([]uint16, volumeEnumerationBufferSize)
	find, err := windows.FindFirstVolume(&buffer[0], uint32(len(buffer)))
	if err != nil {
		return nil, err
	}
	defer windows.FindVolumeClose(find)

	var volumes []string
	for {
		volumes = append(volumes, windows.UTF16ToString(buffer))

		for i := range buffer {
			buffer[i] = 0
		}

		err = windows.FindNextVolume(find, &buffer[0], uint32(len(buffer)))
		if err == nil {
			continue
		}
		if err == windows.ERROR_NO_MORE_FILES {
			break
		}
		return nil, err
	}

	return volumes, nil
}

func openVolume(volume string) (windows.Handle, error) {
	path := strings.TrimRight(volume, `\`)
	return windows.CreateFile(
		windows.StringToUTF16Ptr(path),
		windows.GENERIC_READ|windows.GENERIC_WRITE,
		windows.FILE_SHARE_READ|windows.FILE_SHARE_WRITE,
		nil,
		windows.OPEN_EXISTING,
		0,
		0,
	)
}

func volumeBelongsToDisk(handle windows.Handle, diskNum uint32) (bool, error) {
	out := make([]byte, volumeDiskExtentsBufferSize)
	if err := deviceIoControl(handle, ioctlVolumeGetDiskExtents, out); err != nil {
		return false, err
	}
	if len(out) < volumeDiskExtentHeaderSize {
		return false, fmt.Errorf("disk extents response is too small")
	}

	count := binary.LittleEndian.Uint32(out[:4])
	for i := range count {
		offset := volumeDiskExtentHeaderSize + int(i)*volumeDiskExtentSize
		if offset+volumeDiskExtentSize > len(out) {
			return false, fmt.Errorf("disk extents response is truncated")
		}
		extentDiskNum := binary.LittleEndian.Uint32(out[offset+volumeDiskExtentDiskNumStart:])
		if extentDiskNum == diskNum {
			return true, nil
		}
	}

	return false, nil
}

func deviceIoControl(handle windows.Handle, controlCode uint32, out []byte) error {
	var bytesReturned uint32
	var outPtr *byte
	if len(out) > 0 {
		outPtr = &out[0]
	}
	return windows.DeviceIoControl(handle, controlCode, nil, 0, outPtr, uint32(len(out)), &bytesReturned, nil)
}

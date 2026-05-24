package main

import (
	"fmt"
	"os/exec"
	"runtime"
	"strings"

	log "github.com/charmbracelet/log"
)

func tryUnmountIfNeeded(path string) error {
	if runtime.GOOS == "darwin" {
		isDisk := strings.HasPrefix(path, "/dev/disk")
		if isDisk {
			diskId := strings.TrimPrefix(path, "/dev/")
			log.Infof("Attempting to unmount %s", path)
			cmd := exec.Command("diskutil", "umountDisk", "force", diskId)
			_, err := cmd.Output()
			if err != nil {
				return fmt.Errorf("failed to unmount disk: %w", err)
			}
		}
	}

	return nil
}

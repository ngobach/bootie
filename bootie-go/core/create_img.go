package core

import (
	"fmt"
	"os"

	humanize "github.com/dustin/go-humanize"

	log "github.com/charmbracelet/log"
)

func CreateImg(target, sizeStr string) error {
	sizeBytes, err := ParseSize(sizeStr)
	if err != nil {
		return fmt.Errorf("failed to parse size: %w", err)
	}

	file, err := os.Create(target)
	if err != nil {
		return fmt.Errorf("failed to create image file: %w", err)
	}
	defer file.Close()

	if err := file.Truncate(sizeBytes); err != nil {
		return fmt.Errorf("failed to set image size: %w", err)
	}

	log.Default().Logf(SuccessLevel, "Created raw disk image: %s (%s)", target, humanize.IBytes(uint64(sizeBytes)))
	return nil
}

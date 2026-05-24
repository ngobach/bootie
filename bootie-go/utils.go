package main

import (
	"encoding/binary"
	"fmt"
	"strconv"
	"strings"

	"github.com/google/uuid"
	log "github.com/charmbracelet/log"
)

type diskEntry struct {
	identifier string
	label      string
	size       int64
}

type gptPart struct {
	partitionGUID     uuid.UUID
	firstLBA, lastLBA uint64
	partitionName     string
}

func parseGptPart(buffer []byte) (*gptPart, error) {
	var err error
	part := gptPart{}
	part.partitionGUID, err = uuid.FromBytes(buffer[0:16])

	if err != nil {
		return nil, fmt.Errorf("failed to parse partition GUID: %w", err)
	}

	part.firstLBA = binary.LittleEndian.Uint64(buffer[0x20:0x28])
	part.lastLBA = binary.LittleEndian.Uint64(buffer[0x28:0x30])
	part.partitionName = string(buffer[0x38:0x80])

	return &part, nil
}

func parseSize(sizeStr string) (int64, error) {
	sizeStr = strings.TrimSpace(sizeStr)

	var numStr, unit string
	for i, char := range sizeStr {
		if char >= '0' && char <= '9' {
			continue
		}
		numStr = sizeStr[:i]
		unit = strings.ToUpper(sizeStr[i:])
		break
	}

	if numStr == "" {
		return 0, fmt.Errorf("invalid size format: %s", sizeStr)
	}

	num, err := strconv.ParseInt(numStr, 10, 64)
	if err != nil {
		return 0, fmt.Errorf("failed to parse number: %w", err)
	}

	switch unit {
	case "K", "KB":
		return num * 1024, nil
	case "M", "MB":
		return num * 1024 * 1024, nil
	case "G", "GB":
		return num * 1024 * 1024 * 1024, nil
	case "":
		return num, nil
	default:
		return 0, fmt.Errorf("unsupported unit: %s", unit)
	}
}

func (part *gptPart) isEmpty() bool {
	uuidBytes := part.partitionGUID[:]

	for i := range 16 {
		if uuidBytes[i] != 0 {
			return false
		}
	}

	return true
}

func printGptPart(part *gptPart) {
	log.Debugf("Partition GUID: %s", part.partitionGUID.String())
	log.Debugf("Partition Name: %s", part.partitionName)
	log.Debugf("First LBA: %d", part.firstLBA)
	log.Debugf("Last LBA: %d", part.lastLBA)
}

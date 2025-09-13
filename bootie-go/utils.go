package main

import (
	"encoding/binary"
	"fmt"

	"github.com/google/uuid"
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
	fmt.Printf("Partition GUID: %s\n", part.partitionGUID.String())
	fmt.Printf("Partition Name: %s\n", part.partitionName)
	fmt.Printf("First LBA: %d\n", part.firstLBA)
	fmt.Printf("Last LBA: %d\n", part.lastLBA)
}

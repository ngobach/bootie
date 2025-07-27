package main

import (
	"fmt"
	"os"
)

type GptPartition struct {
}

func cmd_install(target string) error {
	fmt.Println("target", target)
	file, err := os.OpenFile(target, os.O_RDWR, os.ModePerm)

	if err != nil {
		return fmt.Errorf("failed to open target file: %w", err)
	}

	defer file.Close()
	fmt.Println("Installing to", target)

	sectorSize := len(seedSectors)

	if sectorSize%512 != 0 {
		return fmt.Errorf("seedSectors is not a multiple of 512")
	}

	sectorCount := sectorSize / 512

	fmt.Println("Sectors")

	return nil
}

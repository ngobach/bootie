package core

import (
	"os"
	"path/filepath"
	"testing"

	diskfs "github.com/ngobach/go-diskfs"
	diskfsFile "github.com/ngobach/go-diskfs/backend/file"
	"github.com/ngobach/go-diskfs/filesystem"
	"github.com/ngobach/go-diskfs/partition/gpt"
)

func createSparseTestImage(t *testing.T, size int64) string {
	t.Helper()
	dir := t.TempDir()
	path := filepath.Join(dir, "test.img")
	f, err := os.Create(path)
	if err != nil {
		t.Fatalf("failed to create test image: %v", err)
	}
	if err := f.Truncate(size); err != nil {
		f.Close()
		t.Fatalf("failed to truncate test image: %v", err)
	}
	if err := f.Close(); err != nil {
		t.Fatalf("failed to close test image: %v", err)
	}
	return path
}

func TestInitializeDisk_SeparateExFAT(t *testing.T) {
	imgPath := createSparseTestImage(t, 260*1024*1024)

	err := InitializeDisk(imgPath, "separate", "exfat", false)
	if err != nil {
		t.Fatalf("InitializeDisk failed: %v", err)
	}

	backend, err := diskfsFile.OpenFromPath(imgPath, false)
	if err != nil {
		t.Fatalf("OpenFromPath failed: %v", err)
	}
	defer backend.Close()

	disk, err := diskfs.OpenBackend(backend)
	if err != nil {
		t.Fatalf("OpenBackend failed: %v", err)
	}

	gptTable, ok := disk.Table.(*gpt.Table)
	if !ok {
		t.Fatalf("expected *gpt.Table, got %T", disk.Table)
	}
	if len(gptTable.Partitions) != 2 {
		t.Fatalf("expected 2 partitions, got %d", len(gptTable.Partitions))
	}

	// Partition 1: EFI (FAT32)
	efiFs, err := disk.GetFilesystem(1)
	if err != nil {
		t.Fatalf("GetFilesystem(1) failed: %v", err)
	}
	if efiFs.Type() != filesystem.TypeFat32 {
		t.Errorf("expected partition 1 to be TypeFat32, got %v", efiFs.Type())
	}

	// Partition 2: Bootie (exFAT)
	dataFs, err := disk.GetFilesystem(2)
	if err != nil {
		t.Fatalf("GetFilesystem(2) failed: %v", err)
	}
	if dataFs.Type() != filesystem.TypeExFAT {
		t.Errorf("expected partition 2 to be TypeExFAT, got %v", dataFs.Type())
	}

	menuFile, err := dataFs.Open("menu.lst")
	if err != nil {
		t.Fatalf("failed to open menu.lst in exFAT: %v", err)
	}
	defer menuFile.Close()

	menuIniFile, err := dataFs.Open("menu.ini")
	if err != nil {
		t.Fatalf("failed to open menu.ini in exFAT: %v", err)
	}
	defer menuIniFile.Close()
}

func TestInitializeDisk_SeparateFAT32(t *testing.T) {
	imgPath := createSparseTestImage(t, 260*1024*1024)

	err := InitializeDisk(imgPath, "separate", "fat32", false)
	if err != nil {
		t.Fatalf("InitializeDisk failed: %v", err)
	}

	backend, err := diskfsFile.OpenFromPath(imgPath, false)
	if err != nil {
		t.Fatalf("OpenFromPath failed: %v", err)
	}
	defer backend.Close()

	disk, err := diskfs.OpenBackend(backend)
	if err != nil {
		t.Fatalf("OpenBackend failed: %v", err)
	}

	dataFs, err := disk.GetFilesystem(2)
	if err != nil {
		t.Fatalf("GetFilesystem(2) failed: %v", err)
	}
	if dataFs.Type() != filesystem.TypeFat32 {
		t.Errorf("expected partition 2 to be TypeFat32, got %v", dataFs.Type())
	}
}

func TestInitializeDisk_Combined(t *testing.T) {
	imgPath := createSparseTestImage(t, 8*1024*1024)

	err := InitializeDisk(imgPath, "combined", "fat32", false)
	if err != nil {
		t.Fatalf("InitializeDisk failed: %v", err)
	}

	backend, err := diskfsFile.OpenFromPath(imgPath, false)
	if err != nil {
		t.Fatalf("OpenFromPath failed: %v", err)
	}
	defer backend.Close()

	disk, err := diskfs.OpenBackend(backend)
	if err != nil {
		t.Fatalf("OpenBackend failed: %v", err)
	}

	bootieFs, err := disk.GetFilesystem(1)
	if err != nil {
		t.Fatalf("GetFilesystem(1) failed: %v", err)
	}
	if bootieFs.Type() != filesystem.TypeFat32 {
		t.Errorf("expected partition 1 to be TypeFat32, got %v", bootieFs.Type())
	}
}

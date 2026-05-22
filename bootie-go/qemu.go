package main

import (
	"context"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"

	"github.com/urfave/cli/v3"
	"ngobach.com/bootie-go/resources"
)

type FirmwareType int

const (
	FirmwareTypeBIOS FirmwareType = iota
	FirmwareTypeUEFI
	FirmwareTypeARM64
)

type QemuCommand struct {
	target   string
	uefi     bool
	memory   string
	cpus     int
	qemuArgs []string
	cleanup  func()
}

func (cmd *QemuCommand) execute() error {
	// Determine firmware type
	var firmwareType FirmwareType
	if cmd.uefi {
		firmwareType = FirmwareTypeUEFI
	} else {
		firmwareType = FirmwareTypeBIOS
	}

	// Get firmware path and cleanup function
	firmwarePath, cleanup, err := getFirmwarePath(firmwareType)
	if err != nil {
		return fmt.Errorf("failed to get firmware path: %w", err)
	}
	cmd.cleanup = cleanup
	defer cmd.cleanup()

	// Build QEMU command
	qemuCmd, err := cmd.buildQemuCommand(firmwarePath)
	if err != nil {
		return fmt.Errorf("failed to build QEMU command: %w", err)
	}

	// Execute QEMU
	fmt.Printf("Running QEMU with args: %v\n", qemuCmd.Args)
	return qemuCmd.Run()
}

func (cmd *QemuCommand) buildQemuCommand(firmwarePath string) (*exec.Cmd, error) {
	args := []string{
		"-M", "q35",
		"-m", cmd.memory,
		"-smp", fmt.Sprintf("%d", cmd.cpus),
		"-monitor", "none",
		"-boot", "order=dc",
	}

	// Add firmware if specified
	if firmwarePath != "" {
		args = append(args, "-bios", firmwarePath)
	}

	// Add target drive
	if cmd.target != "" {
		// Unmount disk if needed (macOS)
		if err := tryUnmountIfNeeded(cmd.target); err != nil {
			return nil, fmt.Errorf("failed to unmount disk: %w", err)
		}

		args = append(args, "-drive", fmt.Sprintf("file=%s,if=ide,format=raw", cmd.target))
	}

	// Add additional QEMU arguments
	args = append(args, cmd.qemuArgs...)

	// Determine QEMU binary
	qemuBinary := "qemu-system-x86_64"
	if runtime.GOOS == "windows" {
		qemuBinary = "qemu-system-x86_64.exe"
	}

	return exec.Command(qemuBinary, args...), nil
}

type QemuArm64Command struct {
	target   string
	memory   string
	cpus     int
	qemuArgs []string
	cleanup  func()
}

func (cmd *QemuArm64Command) execute() error {
	// Get ARM64 firmware path and cleanup function
	firmwarePath, cleanup, err := getFirmwarePath(FirmwareTypeARM64)
	if err != nil {
		return fmt.Errorf("failed to get ARM64 firmware path: %w", err)
	}
	cmd.cleanup = cleanup
	defer cmd.cleanup()

	// Build QEMU command
	qemuCmd, err := cmd.buildQemuCommand(firmwarePath)
	if err != nil {
		return fmt.Errorf("failed to build QEMU ARM64 command: %w", err)
	}

	// Execute QEMU
	fmt.Printf("Running QEMU ARM64 with args: %v\n", qemuCmd.Args)
	return qemuCmd.Run()
}

func (cmd *QemuArm64Command) buildQemuCommand(firmwarePath string) (*exec.Cmd, error) {
	args := []string{
		"-M", "virt,accel=hvf,highmem=off",
		"-cpu", "cortex-a76",
		"-m", cmd.memory,
		"-smp", fmt.Sprintf("%d", cmd.cpus),
		"-nographic",
		"-monitor", "none",
		"-bios", firmwarePath,
		"-nic", "user,model=virtio-net-pci",
		"-boot", "order=dc,menu=off",
	}

	// Add target drive
	if cmd.target != "" {
		// Unmount disk if needed (macOS)
		if err := tryUnmountIfNeeded(cmd.target); err != nil {
			return nil, fmt.Errorf("failed to unmount disk: %w", err)
		}

		args = append(args, "-drive", fmt.Sprintf("file=%s,if=ide,format=raw", cmd.target))
	}

	// Add additional QEMU arguments
	args = append(args, cmd.qemuArgs...)

	// Determine QEMU binary
	qemuBinary := "qemu-system-aarch64"
	if runtime.GOOS == "windows" {
		qemuBinary = "qemu-system-aarch64.exe"
	}

	return exec.Command(qemuBinary, args...), nil
}

type QemuCreateCommand struct {
	target string
	size   string
}

func (cmd *QemuCreateCommand) execute() error {
	fmt.Printf("Creating QEMU test image: %s\n", cmd.target)

	switch runtime.GOOS {
	case "darwin":
		return cmd.createDarwinImage()
	case "linux":
		return cmd.createLinuxImage()
	default:
		return fmt.Errorf("unsupported OS for image creation: %s", runtime.GOOS)
	}
}

func (cmd *QemuCreateCommand) createDarwinImage() error {
	// Use hdiutil to create a GPT-formatted disk image
	args := []string{
		"create",
		"-size", cmd.size,
		"-layout", "GPTSPUD",
		"-align", "32",
		"-fs", "FAT32",
		"-volname", "EFI",
		cmd.target,
	}

	cmdObj := exec.Command("hdiutil", args...)
	output, err := cmdObj.CombinedOutput()
	if err != nil {
		return fmt.Errorf("failed to create disk image: %w\nOutput: %s", err, string(output))
	}

	// Rename .dmg to target name if needed
	dmgFile := cmd.target + ".dmg"
	if _, err := os.Stat(dmgFile); err == nil {
		if err := os.Rename(dmgFile, cmd.target); err != nil {
			return fmt.Errorf("failed to rename disk image: %w", err)
		}
	}

	fmt.Printf("Successfully created disk image: %s\n", cmd.target)
	return nil
}

func (cmd *QemuCreateCommand) createLinuxImage() error {
	// Use dd to create a raw disk image
	sizeBytes, err := parseSize(cmd.size)
	if err != nil {
		return fmt.Errorf("failed to parse size: %w", err)
	}

	// Create the file with dd
	ddCmd := exec.Command("dd", "if=/dev/zero", fmt.Sprintf("of=%s", cmd.target), "bs=1M", fmt.Sprintf("count=%d", sizeBytes/1024/1024))
	output, err := ddCmd.CombinedOutput()
	if err != nil {
		return fmt.Errorf("failed to create disk image: %w\nOutput: %s", err, string(output))
	}

	// Create GPT partition table
	partedCmd := exec.Command("parted", "-s", cmd.target, "mklabel", "gpt")
	output, err = partedCmd.CombinedOutput()
	if err != nil {
		return fmt.Errorf("failed to create GPT partition table: %w\nOutput: %s", err, string(output))
	}

	// Create EFI partition
	partedCmd = exec.Command("parted", "-s", cmd.target, "mkpart", "EFI", "fat32", "1MiB", "201MiB")
	output, err = partedCmd.CombinedOutput()
	if err != nil {
		return fmt.Errorf("failed to create EFI partition: %w\nOutput: %s", err, string(output))
	}

	// Create data partition
	partedCmd = exec.Command("parted", "-s", cmd.target, "mkpart", "Bootie", "fat32", "201MiB", "100%")
	output, err = partedCmd.CombinedOutput()
	if err != nil {
		return fmt.Errorf("failed to create data partition: %w\nOutput: %s", err, string(output))
	}

	fmt.Printf("Successfully created disk image: %s\n", cmd.target)
	return nil
}

// Helper functions

func getFirmwarePath(firmwareType FirmwareType) (string, func(), error) {
	var firmwareData []byte
	var filename string

	switch firmwareType {
	case FirmwareTypeUEFI:
		firmwareData = resources.BiosBin
		filename = "bios.bin"
	case FirmwareTypeARM64:
		firmwareData = resources.Edk2Aarch64Code
		filename = "edk2-aarch64-code.fd"
	default:
		return "", func() {}, nil
	}

	// Create temporary directory
	tempDir, err := os.MkdirTemp("", "bootie-qemu-*")
	if err != nil {
		return "", nil, fmt.Errorf("failed to create temp directory: %w", err)
	}

	// Create firmware file
	firmwarePath := filepath.Join(tempDir, filename)
	err = os.WriteFile(firmwarePath, firmwareData, 0644)
	if err != nil {
		return "", nil, fmt.Errorf("failed to write firmware file: %w", err)
	}

	// Return cleanup function
	cleanup := func() {
		os.RemoveAll(tempDir)
	}

	return firmwarePath, cleanup, nil
}

// CLI Commands

func createQemuCommand() *cli.Command {
	return &cli.Command{
		Name:  "qemu",
		Usage: "QEMU testing and development commands",
		Commands: []*cli.Command{
			{
				Name:  "run",
				Usage: "Run QEMU with a disk image or device",
				Flags: []cli.Flag{
					&cli.StringFlag{
						Name:    "target",
						Aliases: []string{"t"},
						Usage:   "Target disk device or image file",
					},
					&cli.BoolFlag{
						Name:    "uefi",
						Aliases: []string{"u"},
						Usage:   "Enable UEFI boot mode",
					},
					&cli.StringFlag{
						Name:    "memory",
						Aliases: []string{"m"},
						Value:   "2G",
						Usage:   "Memory size (e.g., 2G, 1024M)",
					},
					&cli.IntFlag{
						Name:    "cpus",
						Aliases: []string{"c"},
						Value:   4,
						Usage:   "Number of CPUs",
					},
				},
				Action: func(c context.Context, cmd *cli.Command) error {
					qemuCmd := &QemuCommand{
						target:   cmd.String("target"),
						uefi:     cmd.Bool("uefi"),
						memory:   cmd.String("memory"),
						cpus:     cmd.Int("cpus"),
						qemuArgs: cmd.Args().Slice(),
					}
					return qemuCmd.execute()
				},
			},
			{
				Name:  "run-arm64",
				Usage: "Run QEMU for ARM64 architecture",
				Flags: []cli.Flag{
					&cli.StringFlag{
						Name:    "target",
						Aliases: []string{"t"},
						Usage:   "Target disk device or image file",
					},
					&cli.StringFlag{
						Name:    "memory",
						Aliases: []string{"m"},
						Value:   "2G",
						Usage:   "Memory size (e.g., 2G, 1024M)",
					},
					&cli.IntFlag{
						Name:    "cpus",
						Aliases: []string{"c"},
						Value:   2,
						Usage:   "Number of CPUs",
					},
				},
				Action: func(c context.Context, cmd *cli.Command) error {
					qemuCmd := &QemuArm64Command{
						target:   cmd.String("target"),
						memory:   cmd.String("memory"),
						cpus:     cmd.Int("cpus"),
						qemuArgs: cmd.Args().Slice(),
					}
					return qemuCmd.execute()
				},
			},
			{
				Name:  "create",
				Usage: "Create a test disk image for QEMU",
				Flags: []cli.Flag{
					&cli.StringFlag{
						Name:     "target",
						Required: true,
						Usage:    "Target image file path",
					},
					&cli.StringFlag{
						Name:    "size",
						Aliases: []string{"s"},
						Value:   "200M",
						Usage:   "Image size (e.g., 200M, 1G)",
					},
				},
				Action: func(c context.Context, cmd *cli.Command) error {
					qemuCmd := &QemuCreateCommand{
						target: cmd.String("target"),
						size:   cmd.String("size"),
					}
					return qemuCmd.execute()
				},
			},
		},
	}
}

package main

import (
	"context"
	"fmt"
	"os"
	"os/exec"
	"runtime"

	"github.com/urfave/cli/v3"

	log "github.com/charmbracelet/log"
)

type QemuCommand struct {
	target   string
	firmware string
	memory   string
	cpus     int
	qemuArgs []string
}

func (cmd *QemuCommand) execute() error {
	if err := validateFirmware(cmd.firmware); err != nil {
		return err
	}

	qemuCmd, err := cmd.buildQemuCommand(cmd.firmware)
	if err != nil {
		return fmt.Errorf("failed to build QEMU command: %w", err)
	}

	log.Infof("Running QEMU with args: %v", qemuCmd.Args)
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
	firmware string
	memory   string
	cpus     int
	qemuArgs []string
}

func (cmd *QemuArm64Command) execute() error {
	if err := validateFirmware(cmd.firmware); err != nil {
		return err
	}

	qemuCmd, err := cmd.buildQemuCommand(cmd.firmware)
	if err != nil {
		return fmt.Errorf("failed to build QEMU ARM64 command: %w", err)
	}

	log.Infof("Running QEMU ARM64 with args: %v", qemuCmd.Args)
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
	log.Infof("Creating QEMU test image: %s", cmd.target)

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

	log.Default().Logf(SuccessLevel, "Successfully created disk image: %s", cmd.target)
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

	log.Default().Logf(SuccessLevel, "Successfully created disk image: %s", cmd.target)
	return nil
}

// Helper functions

func validateFirmware(firmware string) error {
	if firmware == "" {
		return nil
	}
	info, err := os.Stat(firmware)
	if err != nil {
		return fmt.Errorf("firmware file %s: %w", firmware, err)
	}
	if info.IsDir() {
		return fmt.Errorf("firmware path %s is a directory, not a file", firmware)
	}
	return nil
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
					&cli.StringFlag{
						Name:    "firmware",
						Aliases: []string{"f"},
						Usage:   "Path to UEFI firmware file (e.g. /usr/share/ovmf/OVMF.fd)",
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
						firmware: cmd.String("firmware"),
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
						Name:     "firmware",
						Aliases:  []string{"f"},
						Required: true,
						Usage:    "Path to ARM64 firmware file (e.g. /usr/share/qemu-efi-aarch64/QEMU_EFI.fd)",
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
						firmware: cmd.String("firmware"),
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

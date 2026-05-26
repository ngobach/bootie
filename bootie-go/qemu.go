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

type QemuRunCommand struct {
	target   string
	firmware string
	memory   string
	cpus     int
	arch     string
	volume   string
	qemuArgs []string
}

func (cmd *QemuRunCommand) execute() error {
	if err := validateFirmware(cmd.firmware); err != nil {
		return err
	}

	if err := validateVolume(cmd.volume); err != nil {
		return err
	}

	qemuCmd, err := cmd.buildQemuCommand()
	if err != nil {
		return fmt.Errorf("failed to build QEMU command: %w", err)
	}

	log.Infof("Running QEMU (%s) with args: %v", cmd.arch, qemuCmd.Args)
	return qemuCmd.Run()
}

func (cmd *QemuRunCommand) buildQemuCommand() (*exec.Cmd, error) {
	switch cmd.arch {
	case "arm64":
		return cmd.buildArm64()
	default:
		return cmd.buildX86_64()
	}
}

func (cmd *QemuRunCommand) buildX86_64() (*exec.Cmd, error) {
	args := []string{
		"-M", "q35",
		"-m", cmd.memory,
		"-smp", fmt.Sprintf("%d", cmd.cpus),
		"-monitor", "none",
		"-boot", "order=dc",
	}

	if cmd.firmware != "" {
		args = append(args, "-bios", cmd.firmware)
	}

	if err := DefaultDiskManager.LockDisk(cmd.target); err != nil {
		return nil, fmt.Errorf("failed to unmount disk: %w", err)
	}

	args = append(args, "-drive", fmt.Sprintf("file=%s,if=ide,format=raw", cmd.target))

	if cmd.volume != "" {
		args = append(args, "-drive", fmt.Sprintf("file=fat:rw:%s,if=ide", cmd.volume))
	}

	args = append(args, cmd.qemuArgs...)

	qemuBinary := "qemu-system-x86_64"
	if runtime.GOOS == "windows" {
		qemuBinary = "qemu-system-x86_64.exe"
	}

	return exec.Command(qemuBinary, args...), nil
}

func (cmd *QemuRunCommand) buildArm64() (*exec.Cmd, error) {
	args := []string{
		"-M", "virt,accel=hvf,highmem=off",
		"-cpu", "cortex-a76",
		"-m", cmd.memory,
		"-smp", fmt.Sprintf("%d", cmd.cpus),
		"-nographic",
		"-monitor", "none",
		"-bios", cmd.firmware,
		"-nic", "user,model=virtio-net-pci",
		"-boot", "order=dc,menu=off",
	}

	if err := DefaultDiskManager.LockDisk(cmd.target); err != nil {
		return nil, fmt.Errorf("failed to unmount disk: %w", err)
	}

	args = append(args, "-drive", fmt.Sprintf("file=%s,if=ide,format=raw", cmd.target))

	if cmd.volume != "" {
		args = append(args, "-drive", fmt.Sprintf("file=fat:rw:%s,if=ide", cmd.volume))
	}

	args = append(args, cmd.qemuArgs...)

	qemuBinary := "qemu-system-aarch64"
	if runtime.GOOS == "windows" {
		qemuBinary = "qemu-system-aarch64.exe"
	}

	return exec.Command(qemuBinary, args...), nil
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

func validateVolume(volume string) error {
	if volume == "" {
		return nil
	}
	info, err := os.Stat(volume)
	if err != nil {
		return fmt.Errorf("volume path %s: %w", volume, err)
	}
	if !info.IsDir() {
		return fmt.Errorf("volume path %s is not a directory", volume)
	}
	return nil
}

// CLI Commands

func runQemuCommand() *cli.Command {
	return &cli.Command{
		Name:  "run-qemu",
		Usage: "Run QEMU with a disk image or device",
		Flags: []cli.Flag{
			&cli.StringFlag{
				Name:     "target",
				Aliases:  []string{"t"},
				Usage:    "Target disk device or image file",
				Required: true,
			},
			&cli.StringFlag{
				Name:    "volume",
				Aliases: []string{"v"},
				Usage:   "Folder to mount as a virtual FAT disk drive",
			},
			&cli.StringFlag{
				Name:    "firmware",
				Aliases: []string{"f"},
				Usage:   "Path to UEFI firmware file",
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
			&cli.StringFlag{
				Name:    "arch",
				Aliases: []string{"a"},
				Value:   "x86_64",
				Usage:   "Target architecture (x86_64 or arm64)",
			},
		},
		Action: func(c context.Context, cmd *cli.Command) error {
			runCmd := &QemuRunCommand{
				target:   cmd.String("target"),
				firmware: cmd.String("firmware"),
				memory:   cmd.String("memory"),
				cpus:     cmd.Int("cpus"),
				arch:     cmd.String("arch"),
				volume:   cmd.String("volume"),
				qemuArgs: cmd.Args().Slice(),
			}
			return runCmd.execute()
		},
	}
}

package core

import (
	"fmt"
	"os"
	"os/exec"
	"runtime"

	log "github.com/charmbracelet/log"
)

type QemuRunCommand struct {
	Target   string
	Firmware string
	Memory   string
	Cpus     int
	Arch     string
	Volume   string
	QemuArgs []string
}

func (cmd *QemuRunCommand) Execute() error {
	if err := validateFirmware(cmd.Firmware); err != nil {
		return err
	}

	if err := validateVolume(cmd.Volume); err != nil {
		return err
	}

	qemuCmd, err := cmd.buildQemuCommand()
	if err != nil {
		return fmt.Errorf("failed to build QEMU command: %w", err)
	}

	log.Infof("Running QEMU (%s) with args: %v", cmd.Arch, qemuCmd.Args)
	return qemuCmd.Run()
}

func (cmd *QemuRunCommand) buildQemuCommand() (*exec.Cmd, error) {
	switch cmd.Arch {
	case "arm64":
		return cmd.buildArm64()
	default:
		return cmd.buildX86_64()
	}
}

func (cmd *QemuRunCommand) buildX86_64() (*exec.Cmd, error) {
	args := []string{
		"-M", "q35",
		"-m", cmd.Memory,
		"-smp", fmt.Sprintf("%d", cmd.Cpus),
		"-monitor", "none",
		"-boot", "order=dc",
	}

	if cmd.Firmware != "" {
		args = append(args, "-bios", cmd.Firmware)
	}

	if err := DefaultDiskManager.LockDisk(cmd.Target); err != nil {
		return nil, fmt.Errorf("failed to unmount disk: %w", err)
	}

	args = append(args, "-drive", fmt.Sprintf("file=%s,if=ide,format=raw", cmd.Target))

	if cmd.Volume != "" {
		args = append(args, "-drive", fmt.Sprintf("file=fat:rw:%s,if=ide", cmd.Volume))
	}

	args = append(args, cmd.QemuArgs...)

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
		"-m", cmd.Memory,
		"-smp", fmt.Sprintf("%d", cmd.Cpus),
		"-nographic",
		"-monitor", "none",
		"-bios", cmd.Firmware,
		"-nic", "user,model=virtio-net-pci",
		"-boot", "order=dc,menu=off",
	}

	if err := DefaultDiskManager.LockDisk(cmd.Target); err != nil {
		return nil, fmt.Errorf("failed to unmount disk: %w", err)
	}

	args = append(args, "-drive", fmt.Sprintf("file=%s,if=ide,format=raw", cmd.Target))

	if cmd.Volume != "" {
		args = append(args, "-drive", fmt.Sprintf("file=fat:rw:%s,if=ide", cmd.Volume))
	}

	args = append(args, cmd.QemuArgs...)

	qemuBinary := "qemu-system-aarch64"
	if runtime.GOOS == "windows" {
		qemuBinary = "qemu-system-aarch64.exe"
	}

	return exec.Command(qemuBinary, args...), nil
}

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

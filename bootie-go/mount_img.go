package main

import (
	"context"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/urfave/cli/v3"
	"howett.net/plist"

	log "github.com/charmbracelet/log"
)

type hdiutilInfo struct {
	Images []hdiutilImage `plist:"images"`
}

type hdiutilImage struct {
	ImagePath      string                `plist:"image-path"`
	SystemEntities []hdiutilSystemEntity `plist:"system-entities"`
}

type hdiutilSystemEntity struct {
	DevEntry string `plist:"dev-entry"`
}

func mountImage(target string) (string, error) {
	info, err := os.Stat(target)
	if err != nil {
		return "", fmt.Errorf("image file %s: %w", target, err)
	}
	if info.IsDir() {
		return "", fmt.Errorf("%s is a directory, not a disk image", target)
	}

	switch runtime.GOOS {
	case "linux":
		return mountImageLinux(target)
	case "darwin":
		return mountImageMac(target)
	case "windows":
		return mountImageWindows(target)
	default:
		return "", fmt.Errorf("unsupported OS: %s", runtime.GOOS)
	}
}

func mountImageLinux(target string) (string, error) {
	cmd := exec.Command("losetup", "-fP", "--show", target)
	out, err := cmd.Output()
	if err != nil {
		return "", fmt.Errorf("losetup failed: %w", err)
	}
	return strings.TrimSpace(string(out)), nil
}

func mountImageMac(target string) (string, error) {
	cmd := exec.Command("hdiutil", "attach", "-nomount", target)
	out, err := cmd.Output()
	if err != nil {
		return "", fmt.Errorf("hdiutil failed: %w", err)
	}
	return strings.TrimSpace(string(out)), nil
}

func mountImageWindows(target string) (string, error) {
	// ImDisk must be installed: https://sourceforge.net/projects/imdisk-toolkit/
	cmd := exec.Command("imdisk", "-a", "-f", target, "-o", "ro")
	out, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("imdisk failed (is ImDisk Toolkit installed?): %w\n%s", err, string(out))
	}
	return strings.TrimSpace(string(out)), nil
}

func unmountImage(target string) error {
	target, err := filepath.Abs(target)
	if err != nil {
		return err
	}

	switch runtime.GOOS {
	case "linux":
		return unmountImageLinux(target)
	case "darwin":
		return unmountImageMac(target)
	case "windows":
		return unmountImageWindows(target)
	default:
		return fmt.Errorf("unsupported OS: %s", runtime.GOOS)
	}
}

func unmountImageLinux(target string) error {
	cmd := exec.Command("losetup", "-j", target)
	out, err := cmd.Output()
	if err != nil {
		return fmt.Errorf("failed to find loop device for %s: %w", target, err)
	}
	line := strings.TrimSpace(string(out))
	if line == "" {
		return fmt.Errorf("no loop device found for %s", target)
	}
	device := strings.SplitN(line, ":", 2)[0]
	return exec.Command("losetup", "-d", device).Run()
}

func unmountImageMac(target string) error {
	cmd := exec.Command("hdiutil", "info", "-plist")
	out, err := cmd.Output()
	if err != nil {
		return fmt.Errorf("failed to get hdiutil info: %w", err)
	}
	var info hdiutilInfo
	if _, err := plist.Unmarshal(out, &info); err != nil {
		return fmt.Errorf("failed to parse hdiutil plist: %w", err)
	}
	for _, img := range info.Images {
		if img.ImagePath == target {
			for _, entity := range img.SystemEntities {
				if entity.DevEntry != "" {
					return exec.Command("hdiutil", "detach", entity.DevEntry).Run()
				}
			}
		}
	}
	return fmt.Errorf("no image found for %s", target)
}

func unmountImageWindows(target string) error {
	cmd := exec.Command("imdisk", "-l")
	out, err := cmd.Output()
	if err != nil {
		return fmt.Errorf("failed to list imdisk devices: %w", err)
	}
	for _, line := range strings.Split(string(out), "\n") {
		if strings.Contains(line, target) {
			parts := strings.SplitN(line, " -> ", 2)
			if len(parts) == 2 {
				return exec.Command("imdisk", "-D", "-m", strings.TrimSpace(parts[0])).Run()
			}
		}
	}
	return fmt.Errorf("no imdisk device found for %s", target)
}

func mountImgCommand() *cli.Command {
	return &cli.Command{
		Name:  "mount-img",
		Usage: "Mount a raw disk image as a virtual disk device",
		Flags: []cli.Flag{
			&cli.StringFlag{
				Name:     "target",
				Aliases:  []string{"t"},
				Required: true,
				Usage:    "Path to the raw disk image file",
			},
			&cli.BoolFlag{
				Name:    "unmount",
				Aliases: []string{"u"},
				Usage:   "Unmount the virtual drive instead of mounting",
			},
		},
		Action: func(_ context.Context, cmd *cli.Command) error {
			target := cmd.String("target")
			if cmd.Bool("unmount") {
				if err := unmountImage(target); err != nil {
					return err
				}
				log.Default().Logf(SuccessLevel, "Unmounted %s", target)
				return nil
			}
			device, err := mountImage(target)
			if err != nil {
				return err
			}
			log.Default().Logf(SuccessLevel, "Mounted at %s", device)
			fmt.Println(device)
			return nil
		},
	}
}

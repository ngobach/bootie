package core

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"

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

func MountImage(target string) (string, error) {
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
	lines := strings.Split(strings.TrimSpace(string(out)), "\n")
	if len(lines) > 0 {
		fields := strings.Fields(lines[0])
		if len(fields) > 0 {
			return fields[0], nil
		}
	}
	return strings.TrimSpace(string(out)), nil
}

func checkImDisk() error {
	if _, err := exec.LookPath("imdisk"); err != nil {
		url := "https://sourceforge.net/projects/imdisk-toolkit/"
		log.Warn("ImDisk Toolkit is not installed but is required on Windows.")
		log.Warnf("Opening download page: %s", url)
		_ = openBrowser(url)
		return fmt.Errorf("imdisk is required but was not found in PATH; please install ImDisk Toolkit from %s", url)
	}
	return nil
}

func openBrowser(url string) error {
	return exec.Command("cmd", "/c", "start", "", url).Start()
}

func mountImageWindows(target string) (string, error) {
	if err := checkImDisk(); err != nil {
		return "", err
	}
	cmd := exec.Command("imdisk", "-a", "-f", target, "-o", "ro")
	out, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("imdisk failed: %w\n%s", err, string(out))
	}
	return strings.TrimSpace(string(out)), nil
}

func UnmountImage(target string) error {
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
	if err := checkImDisk(); err != nil {
		return err
	}
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

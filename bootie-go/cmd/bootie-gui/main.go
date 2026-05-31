package main

import (
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"
	"time"

	humanize "github.com/dustin/go-humanize"
	. "modernc.org/tk9.0"

	log "github.com/charmbracelet/log"
	"ngobach.com/bootie-go/core"
)

func main() {
	// 1. Initialise the App Window
	App.WmTitle("Bootie")
	App.SetResizable(false, false)

	// Create main container frame
	mainFrame := TFrame()
	Pack(mainFrame, Fill("both"), Expand(true), Padx("20"), Pady("20"))

	// 2. Setup Timestamped Temp Log File
	tempDir := os.TempDir()
	timestamp := time.Now().Format("20060102-150405")
	logFilename := filepath.Join(tempDir, fmt.Sprintf("bootie-%s.log", timestamp))
	logFile, err := os.Create(logFilename)
	if err == nil {
		log.SetOutput(io.MultiWriter(os.Stderr, logFile))
	} else {
		log.Errorf("Failed to create log file: %v", err)
	}

	// 3. Create the Log Path Label at the bottom
	logPathLabel := mainFrame.TLabel(Txt(fmt.Sprintf("Logs: %s", logFilename)), Font("Helvetica 9"), Wraplength(360))
	Pack(logPathLabel, Side("bottom"), Fill("x"), Pady("5"), Anchor("w"))

	// 4. Create Content Widgets directly inside mainFrame
	Pack(mainFrame.TLabel(Txt("Format a raw disk or device, installing UEFI EFI and data partitions."), Font("Helvetica 11 bold"), Wraplength(360)), Anchor("w"), Pady("10"))

	diskSelectRow := mainFrame.TFrame()
	Pack(diskSelectRow, Fill("x"), Pady("5"))
	Pack(diskSelectRow.TLabel(Txt("Target Disk: "), Width(12)), Side("left"), Anchor("w"))

	initDiskCombo := diskSelectRow.TCombobox(State("readonly"), Textvariable("initDiskVar"), Width(24))
	Pack(initDiskCombo, Side("left"), Fill("x"), Expand(true), Padx("5"))

	fsRow := mainFrame.TFrame()
	Pack(fsRow, Fill("x"), Pady("5"))
	Pack(fsRow.TLabel(Txt("Filesystem: "), Width(12)), Side("left"), Anchor("w"))
	initFsCombo := fsRow.TCombobox(State("readonly"), Values("exfat fat32"), Textvariable("initFsVar"), Width(10))
	Pack(initFsCombo, Side("left"), Padx("5"))
	initFsCombo.Current(0)

	checkRow := mainFrame.TFrame()
	Pack(checkRow, Fill("x"), Pady("5"))
	noDataVar := Variable(false)
	noDataCheck := checkRow.TCheckbutton(Txt("Skip data partition (no-data-part)"), noDataVar)
	Pack(noDataCheck, Side("left"), Padx("5"))

	btnRow := mainFrame.TFrame()
	Pack(btnRow, Fill("x"), Pady("20"))
	initActionBtn := btnRow.TButton(Txt("Initialize Selected Disk"))
	Pack(initActionBtn, Side("left"))

	// Global disk refresh helper
	refreshAllDisks := func() {
		disks, err := core.DefaultDiskManager.ScanDisks()
		if err != nil {
			log.Errorf("Failed to scan disks: %v", err)
			return
		}

		var vals []string
		for _, disk := range disks {
			sizeStr := humanize.IBytes(uint64(disk.Size))
			vals = append(vals, fmt.Sprintf("{%s - %s (%s)}", disk.Identifier, disk.Label, sizeStr))
		}

		valStr := strings.Join(vals, " ")
		initDiskCombo.Configure(Values(valStr))
		if len(vals) > 0 {
			initDiskCombo.Current(0)
		}
		log.Infof("Refreshed physical disks list (%d found)", len(disks))
	}

	initRefreshBtn := diskSelectRow.TButton(Txt("Refresh List"), Command(refreshAllDisks))
	Pack(initRefreshBtn, Side("right"), Padx("5"))

	getSelectedDiskPath := func(combo *TComboboxWidget) string {
		val := combo.Textvariable()
		if val == "" {
			return ""
		}
		parts := strings.Split(val, " - ")
		if len(parts) > 0 {
			return strings.TrimSpace(parts[0])
		}
		return val
	}

	initActionBtn.Configure(Command(func() {
		target := getSelectedDiskPath(initDiskCombo)
		if target == "" {
			log.Error("No target disk selected.")
			return
		}
		fsType := initFsCombo.Textvariable()
		noDataPart := noDataVar.Get() == "1"

		initActionBtn.Configure(State("disabled"))
		go func() {
			defer PostEvent(func() { initActionBtn.Configure(State("normal")) }, false)
			log.Infof("Starting disk initialization on target: %s (fs: %s, skip-data: %v)...", target, fsType, noDataPart)
			if err := core.InitializeDisk(target, fsType, noDataPart); err != nil {
				log.Errorf("Initialization failed: %v", err)
			}
		}()
	}))

	// Initial scanning trigger
	refreshAllDisks()

	// 5. Start Event Loop
	App.Wait()
}

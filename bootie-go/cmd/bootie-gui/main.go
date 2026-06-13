package main

import (
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"time"

	humanize "github.com/dustin/go-humanize"
	. "modernc.org/tk9.0"

	log "github.com/charmbracelet/log"
	"ngobach.com/bootie-go/core"
)

func openFolder(dir string) {
	var cmd *exec.Cmd
	switch runtime.GOOS {
	case "windows":
		cmd = exec.Command("explorer", dir)
	case "darwin":
		cmd = exec.Command("open", dir)
	default:
		cmd = exec.Command("xdg-open", dir)
	}
	if err := cmd.Start(); err != nil {
		log.Errorf("Failed to open containing folder: %v", err)
	}
}

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

	// 3. Create the Log Path and Open Containing Folder Button at the bottom
	openFolderBtn := mainFrame.TButton(Txt("Open containing folder"), Command(func() {
		openFolder(filepath.Dir(logFilename))
	}))
	Pack(openFolderBtn, Side("bottom"), Anchor("w"), Pady("5"))

	logPathLabel := mainFrame.TLabel(Txt(fmt.Sprintf("Logs: %s", logFilename)), Font("Helvetica 9"))
	Pack(logPathLabel, Side("bottom"), Anchor("w"), Pady("2"))

	// 4. Create Content Widgets directly inside mainFrame
	Pack(mainFrame.TLabel(Txt("Format a raw disk or device, installing UEFI EFI and data partitions."), Font("Helvetica 11 bold"), Wraplength(360)), Anchor("w"), Pady("10"))

	// Target picker components
	targetTypeRow := mainFrame.TFrame()
	Pack(targetTypeRow, Fill("x"), Pady("5"))
	Pack(targetTypeRow.TLabel(Txt("Target Type: "), Width(18)), Side("left"), Anchor("w"))

	targetTypeVar := Variable("discovered")

	var updateTargetState func()

	radioDiscovered := targetTypeRow.TRadiobutton(Txt("Discovered Target"), targetTypeVar, Value("discovered"), Command(func() { updateTargetState() }))
	Pack(radioDiscovered, Side("left"), Padx("5"))

	radioFile := targetTypeRow.TRadiobutton(Txt("Disk Image (File)"), targetTypeVar, Value("file"), Command(func() { updateTargetState() }))
	Pack(radioFile, Side("left"), Padx("5"))

	diskSelectRow := mainFrame.TFrame()
	Pack(diskSelectRow, Fill("x"), Pady("5"))
	Pack(diskSelectRow.TLabel(Txt("Target Disk: "), Width(18)), Side("left"), Anchor("w"))

	initDiskCombo := diskSelectRow.TCombobox(State("readonly"), Textvariable("initDiskVar"), Width(24))
	Pack(initDiskCombo, Side("left"), Fill("x"), Expand(true), Padx("5"))

	fileSelectRow := mainFrame.TFrame()
	Pack(fileSelectRow, Fill("x"), Pady("5"))
	Pack(fileSelectRow.TLabel(Txt("Image File: "), Width(18)), Side("left"), Anchor("w"))

	imageEntry := fileSelectRow.TEntry(Width(24))
	Pack(imageEntry, Side("left"), Fill("x"), Expand(true), Padx("5"))

	browseFile := func() {
		files := GetOpenFile(
			Title("Select Disk Image File"),
			Filetypes([]FileType{
				{TypeName: "Disk Images", Extensions: []string{".img", ".iso", ".raw"}},
				{TypeName: "All Files", Extensions: []string{"*"}},
			}),
		)
		if len(files) > 0 && files[0] != "" {
			imageEntry.Configure(Textvariable(files[0]))
		}
	}

	browseBtn := fileSelectRow.TButton(Txt("Browse..."), Command(browseFile))
	Pack(browseBtn, Side("right"), Padx("5"))

	layoutRow := mainFrame.TFrame()
	Pack(layoutRow, Fill("x"), Pady("5"))
	Pack(layoutRow.TLabel(Txt("Disk layout: "), Width(18)), Side("left"), Anchor("w"))

	initLayoutVar := Variable("separate")

	radioSeparate := layoutRow.TRadiobutton(Txt("Separate (EFI + data)"), initLayoutVar, Value("separate"))
	Pack(radioSeparate, Side("left"), Padx("5"))

	radioCombined := layoutRow.TRadiobutton(Txt("Combined (single FAT32)"), initLayoutVar, Value("combined"))
	Pack(radioCombined, Side("left"), Padx("5"))

	fsRow := mainFrame.TFrame()
	Pack(fsRow, Fill("x"), Pady("5"))
	Pack(fsRow.TLabel(Txt("Data partition FS: "), Width(18)), Side("left"), Anchor("w"))

	initFsVar := Variable("exfat")

	radioExfat := fsRow.TRadiobutton(Txt("exFAT"), initFsVar, Value("exfat"))
	Pack(radioExfat, Side("left"), Padx("5"))

	radioFat32 := fsRow.TRadiobutton(Txt("FAT32"), initFsVar, Value("fat32"))
	Pack(radioFat32, Side("left"), Padx("5"))

	updateFsState := func() {
		if initLayoutVar.Get() == "combined" {
			radioExfat.Configure(State("disabled"))
			radioFat32.Configure(State("disabled"))
		} else {
			radioExfat.Configure(State("normal"))
			radioFat32.Configure(State("normal"))
		}
	}

	radioSeparate.Configure(Command(updateFsState))
	radioCombined.Configure(Command(updateFsState))

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

	updateTargetState = func() {
		targetType := targetTypeVar.Get()
		if targetType == "discovered" {
			initDiskCombo.Configure(State("readonly"))
			initRefreshBtn.Configure(State("normal"))
			imageEntry.Configure(State("disabled"))
			browseBtn.Configure(State("disabled"))
		} else {
			initDiskCombo.Configure(State("disabled"))
			initRefreshBtn.Configure(State("disabled"))
			imageEntry.Configure(State("normal"))
			browseBtn.Configure(State("normal"))
		}
	}

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
		var target string
		if targetTypeVar.Get() == "discovered" {
			target = getSelectedDiskPath(initDiskCombo)
			if target == "" {
				log.Error("No target disk selected.")
				return
			}
		} else {
			target = imageEntry.Textvariable()
			if target == "" {
				log.Error("No disk image file selected.")
				return
			}
		}
		layout := initLayoutVar.Get()
		fsType := initFsVar.Get()

		initActionBtn.Configure(State("disabled"))
		go func() {
			defer PostEvent(func() { initActionBtn.Configure(State("normal")) }, false)
			log.Infof("Starting disk initialization on target: %s (layout: %s, fs: %s)...", target, layout, fsType)
			if err := core.InitializeDisk(target, layout, fsType); err != nil {
				log.Errorf("Initialization failed: %v", err)
			}
		}()
	}))

	// Initial scanning and state trigger
	refreshAllDisks()
	updateTargetState()

	// 5. Start Event Loop
	App.Wait()
}

package main

import (
	"fmt"
	"io"
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"time"

	humanize "github.com/dustin/go-humanize"
	. "modernc.org/tk9.0"

	log "github.com/charmbracelet/log"
	"ngobach.com/bootie-go/core"
	"ngobach.com/bootie-go/resources"
)

func main() {
	// 1. Initialise the App Window
	App.WmTitle("Bootie")
	App.SetResizable(false, false)

	// Create main container frame
	mainFrame := TFrame()
	Pack(mainFrame, Fill("both"), Expand(true), Padx("10"), Pady("10"))

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
	logPathLabel := mainFrame.TLabel(Txt(fmt.Sprintf("Logs written to: %s", logFilename)), Font("Helvetica 9"))
	Pack(logPathLabel, Side("bottom"), Fill("x"), Pady("5"), Anchor("w"))

	// 4. Create the Notebook (Tabs) Container
	nb := mainFrame.TNotebook()
	Pack(nb, Side("top"), Fill("both"), Expand(true))

	// ============================================
	// TAB 1: INITIALIZE DISK
	// ============================================
	initTab := nb.TFrame()
	nb.Add(initTab, Txt("Initialize Disk"))

	initContent := initTab.TFrame()
	Pack(initContent, Fill("both"), Expand(true), Padx("20"), Pady("20"))

	Pack(initContent.TLabel(Txt("Format a raw disk or device, installing UEFI EFI and data partitions."), Font("Helvetica 11 bold")), Anchor("w"), Pady("10"))

	diskSelectRow := initContent.TFrame()
	Pack(diskSelectRow, Fill("x"), Pady("5"))
	Pack(diskSelectRow.TLabel(Txt("Select Target Disk: "), Width(18)), Side("left"), Anchor("w"))

	initDiskCombo := diskSelectRow.TCombobox(State("readonly"), Textvariable("initDiskVar"), Width(50))
	Pack(initDiskCombo, Side("left"), Fill("x"), Expand(true), Padx("5"))

	fsRow := initContent.TFrame()
	Pack(fsRow, Fill("x"), Pady("5"))
	Pack(fsRow.TLabel(Txt("Filesystem type: "), Width(18)), Side("left"), Anchor("w"))
	initFsCombo := fsRow.TCombobox(State("readonly"), Values("exfat fat32"), Textvariable("initFsVar"), Width(20))
	Pack(initFsCombo, Side("left"), Padx("5"))
	initFsCombo.Current(0)

	checkRow := initContent.TFrame()
	Pack(checkRow, Fill("x"), Pady("5"))
	noDataVar := Variable(false)
	noDataCheck := checkRow.TCheckbutton(Txt("Skip formatting and extracting data partition (no-data-part)"), noDataVar)
	Pack(noDataCheck, Side("left"), Padx("5"))

	btnRow := initContent.TFrame()
	Pack(btnRow, Fill("x"), Pady("20"))
	initActionBtn := btnRow.TButton(Txt("Initialize Selected Disk"))
	Pack(initActionBtn, Side("left"))

	// ============================================
	// TAB 2: INSTALL BOOT SECTORS
	// ============================================
	installTab := nb.TFrame()
	nb.Add(installTab, Txt("Install Boot"))

	installContent := installTab.TFrame()
	Pack(installContent, Fill("both"), Expand(true), Padx("20"), Pady("20"))

	Pack(installContent.TLabel(Txt("Install bootie boot sectors and MBR directly on a physical disk."), Font("Helvetica 11 bold")), Anchor("w"), Pady("10"))

	installDiskRow := installContent.TFrame()
	Pack(installDiskRow, Fill("x"), Pady("5"))
	Pack(installDiskRow.TLabel(Txt("Select Target Disk: "), Width(18)), Side("left"), Anchor("w"))

	installDiskCombo := installDiskRow.TCombobox(State("readonly"), Textvariable("installDiskVar"), Width(50))
	Pack(installDiskCombo, Side("left"), Fill("x"), Expand(true), Padx("5"))

	installBtnRow := installContent.TFrame()
	Pack(installBtnRow, Fill("x"), Pady("20"))
	installActionBtn := installBtnRow.TButton(Txt("Install Boot Sectors"))
	Pack(installActionBtn, Side("left"))

	// Global disk refresh helper
	combosToRefresh := []*TComboboxWidget{initDiskCombo, installDiskCombo}
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
		for _, combo := range combosToRefresh {
			combo.Configure(Values(valStr))
			if len(vals) > 0 {
				combo.Current(0)
			}
		}
		log.Infof("Refreshed physical disks list (%d found)", len(disks))
	}

	initRefreshBtn := diskSelectRow.TButton(Txt("Refresh List"), Command(refreshAllDisks))
	Pack(initRefreshBtn, Side("right"), Padx("5"))

	installRefreshBtn := installDiskRow.TButton(Txt("Refresh List"), Command(refreshAllDisks))
	Pack(installRefreshBtn, Side("right"), Padx("5"))

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

	installActionBtn.Configure(Command(func() {
		target := getSelectedDiskPath(installDiskCombo)
		if target == "" {
			log.Error("No target disk selected.")
			return
		}

		installActionBtn.Configure(State("disabled"))
		go func() {
			defer PostEvent(func() { installActionBtn.Configure(State("normal")) }, false)
			log.Infof("Starting boot sectors installation on target: %s...", target)
			if err := core.InstallTo(target); err != nil {
				log.Errorf("Installation failed: %v", err)
			}
		}()
	}))

	// ============================================
	// TAB 3: FILE COPIER
	// ============================================
	copierTab := nb.TFrame()
	nb.Add(copierTab, Txt("File Copier"))

	copierContent := copierTab.TFrame()
	Pack(copierContent, Fill("both"), Expand(true), Padx("20"), Pady("20"))

	Pack(copierContent.TLabel(Txt("Extract embedded resources (EFI / Data assets) directly into any target directory."), Font("Helvetica 11 bold")), Anchor("w"), Pady("10"))

	destRow := copierContent.TFrame()
	Pack(destRow, Fill("x"), Pady("5"))
	Pack(destRow.TLabel(Txt("Target Folder Path: "), Width(18)), Side("left"), Anchor("w"))
	destEntry := destRow.TEntry(Textvariable("destVar"), Width(50))
	Pack(destEntry, Side("left"), Fill("x"), Expand(true), Padx("5"))

	copierButtonsRow := copierContent.TFrame()
	Pack(copierButtonsRow, Fill("x"), Pady("20"))

	copyEfiBtn := copierButtonsRow.TButton(Txt("Extract EFI Files"))
	Pack(copyEfiBtn, Side("left"), Padx("5"))

	copyDataBtn := copierButtonsRow.TButton(Txt("Extract Data Files"))
	Pack(copyDataBtn, Side("left"), Padx("5"))

	copyEfiBtn.Configure(Command(func() {
		dest := destEntry.Textvariable()
		if dest == "" {
			log.Error("Destination path must not be empty.")
			return
		}
		copyEfiBtn.Configure(State("disabled"))
		go func() {
			defer PostEvent(func() { copyEfiBtn.Configure(State("normal")) }, false)
			log.Infof("Extracting EFI files to directory: %s...", dest)
			err := core.CopyToLocalFilesystem(&resources.EfiFiles, "efi-part", dest)
			if err != nil {
				log.Errorf("EFI extraction failed: %v", err)
			}
		}()
	}))

	copyDataBtn.Configure(Command(func() {
		dest := destEntry.Textvariable()
		if dest == "" {
			log.Error("Destination path must not be empty.")
			return
		}
		copyDataBtn.Configure(State("disabled"))
		go func() {
			defer PostEvent(func() { copyDataBtn.Configure(State("normal")) }, false)
			log.Infof("Extracting Data files to directory: %s...", dest)
			err := core.CopyToLocalFilesystem(&resources.DataFiles, "data-part", dest)
			if err != nil {
				log.Errorf("Data extraction failed: %v", err)
			}
		}()
	}))

	// ============================================
	// TAB 4: QEMU VIRTUALIZATION
	// ============================================
	qemuTab := nb.TFrame()
	nb.Add(qemuTab, Txt("QEMU Emulator"))

	qemuContent := qemuTab.TFrame()
	Pack(qemuContent, Fill("both"), Expand(true), Padx("20"), Pady("20"))

	Pack(qemuContent.TLabel(Txt("Configure and spin up QEMU emulator testing your setup virtualized."), Font("Helvetica 11 bold")), Anchor("w"), Pady("10"))

	qemuGrid := qemuContent.TFrame()
	Pack(qemuGrid, Fill("x"), Pady("5"))

	makeQemuRow := func(label string, varName string, defaultVal string) *TEntryWidget {
		row := qemuGrid.TFrame()
		Pack(row, Fill("x"), Pady("3"))
		Pack(row.TLabel(Txt(label), Width(18)), Side("left"), Anchor("w"))
		entry := row.TEntry(Textvariable(varName))
		Pack(entry, Side("left"), Fill("x"), Expand(true), Padx("5"))
		if defaultVal != "" {
			entry.Configure(Textvariable(defaultVal))
		}
		return entry
	}

	qemuTarget := makeQemuRow("Target Image/Disk: ", "qemuTargetVar", "")
	qemuVolume := makeQemuRow("Virtual Volume Dir: ", "qemuVolumeVar", "")
	qemuFirmware := makeQemuRow("UEFI Firmware Path: ", "qemuFirmwareVar", "")
	qemuMemory := makeQemuRow("RAM Size (Memory): ", "qemuMemoryVar", "2G")
	qemuCpus := makeQemuRow("CPUs Allocation: ", "qemuCpusVar", "4")

	qemuArchRow := qemuGrid.TFrame()
	Pack(qemuArchRow, Fill("x"), Pady("3"))
	Pack(qemuArchRow.TLabel(Txt("Target Architecture: "), Width(18)), Side("left"), Anchor("w"))

	defaultArch := "x86_64"
	if runtime.GOARCH == "arm64" && runtime.GOOS == "darwin" {
		defaultArch = "arm64"
	}
	qemuArchCombo := qemuArchRow.TCombobox(State("readonly"), Values("x86_64 arm64"), Textvariable("qemuArchVar"), Width(10))
	Pack(qemuArchCombo, Side("left"), Padx("5"))
	if defaultArch == "arm64" {
		qemuArchCombo.Current(1)
	} else {
		qemuArchCombo.Current(0)
	}

	qemuBtnRow := qemuContent.TFrame()
	Pack(qemuBtnRow, Fill("x"), Pady("20"))

	qemuActionBtn := qemuBtnRow.TButton(Txt("Launch QEMU Emulator"))
	Pack(qemuActionBtn, Side("left"))

	qemuActionBtn.Configure(Command(func() {
		target := qemuTarget.Textvariable()
		if target == "" {
			log.Error("QEMU Target Image/Disk path is required.")
			return
		}

		var cpusInt int
		fmt.Sscanf(qemuCpus.Textvariable(), "%d", &cpusInt)
		if cpusInt <= 0 {
			cpusInt = 4
		}

		runCmd := &core.QemuRunCommand{
			Target:   target,
			Volume:   qemuVolume.Textvariable(),
			Firmware: qemuFirmware.Textvariable(),
			Memory:   qemuMemory.Textvariable(),
			Cpus:     cpusInt,
			Arch:     qemuArchCombo.Textvariable(),
		}

		qemuActionBtn.Configure(State("disabled"))
		go func() {
			defer PostEvent(func() { qemuActionBtn.Configure(State("normal")) }, false)
			log.Info("Launching virtual environment QEMU...")
			if err := runCmd.Execute(); err != nil {
				log.Errorf("QEMU Execution failed: %v", err)
			}
		}()
	}))

	// ============================================
	// TAB 5: DISK IMAGES (Creation and Mount)
	// ============================================
	imagesTab := nb.TFrame()
	nb.Add(imagesTab, Txt("Raw Images"))

	imagesContent := imagesTab.TFrame()
	Pack(imagesContent, Fill("both"), Expand(true), Padx("20"), Pady("20"))

	createFrame := imagesContent.TLabelframe(Txt("Create Raw Disk Image File"))
	Pack(createFrame, Fill("x"), Pady("10"), Padx("5"), Ipady("10"))

	cPathRow := createFrame.TFrame()
	Pack(cPathRow, Fill("x"), Pady("3"), Padx("10"))
	Pack(cPathRow.TLabel(Txt("Output File Path: "), Width(18)), Side("left"), Anchor("w"))
	createPath := cPathRow.TEntry(Textvariable("createPathVar"))
	Pack(createPath, Side("left"), Fill("x"), Expand(true), Padx("5"))

	cSizeRow := createFrame.TFrame()
	Pack(cSizeRow, Fill("x"), Pady("3"), Padx("10"))
	Pack(cSizeRow.TLabel(Txt("Image Size: "), Width(18)), Side("left"), Anchor("w"))
	createSize := cSizeRow.TEntry(Textvariable("createSizeVar"), Width(20))
	Pack(createSize, Side("left"), Padx("5"))
	createSize.Configure(Textvariable("200M"))

	createBtnRow := createFrame.TFrame()
	Pack(createBtnRow, Fill("x"), Pady("10"), Padx("10"))
	createActionBtn := createBtnRow.TButton(Txt("Create Empty Disk Image"))
	Pack(createActionBtn, Side("left"))

	createActionBtn.Configure(Command(func() {
		target := createPath.Textvariable()
		if target == "" {
			log.Error("Output Image Path is required.")
			return
		}
		sizeStr := createSize.Textvariable()

		createActionBtn.Configure(State("disabled"))
		go func() {
			defer PostEvent(func() { createActionBtn.Configure(State("normal")) }, false)
			log.Infof("Creating raw disk image file: %s (size: %s)...", target, sizeStr)
			if err := core.CreateImg(target, sizeStr); err != nil {
				log.Errorf("Image creation failed: %v", err)
			}
		}()
	}))

	mountFrame := imagesContent.TLabelframe(Txt("Mount / Unmount Disk Image"))
	Pack(mountFrame, Fill("x"), Pady("10"), Padx("5"), Ipady("10"))

	mPathRow := mountFrame.TFrame()
	Pack(mPathRow, Fill("x"), Pady("3"), Padx("10"))
	Pack(mPathRow.TLabel(Txt("Image File Path: "), Width(18)), Side("left"), Anchor("w"))
	mountPath := mPathRow.TEntry(Textvariable("mountPathVar"))
	Pack(mountPath, Side("left"), Fill("x"), Expand(true), Padx("5"))

	mountBtnRow := mountFrame.TFrame()
	Pack(mountBtnRow, Fill("x"), Pady("10"), Padx("10"))

	mountActionBtn := mountBtnRow.TButton(Txt("Mount Image"))
	Pack(mountActionBtn, Side("left"), Padx("5"))

	unmountActionBtn := mountBtnRow.TButton(Txt("Unmount Image"))
	Pack(unmountActionBtn, Side("left"), Padx("5"))

	mountActionBtn.Configure(Command(func() {
		target := mountPath.Textvariable()
		if target == "" {
			log.Error("Image File Path is required.")
			return
		}

		mountActionBtn.Configure(State("disabled"))
		go func() {
			defer PostEvent(func() { mountActionBtn.Configure(State("normal")) }, false)
			log.Infof("Mounting disk image: %s...", target)
			device, err := core.MountImage(target)
			if err != nil {
				log.Errorf("Mount failed: %v", err)
				return
			}
			log.Default().Logf(core.SuccessLevel, "Mounted successfully at virtual loop device: %s", device)
		}()
	}))

	unmountActionBtn.Configure(Command(func() {
		target := mountPath.Textvariable()
		if target == "" {
			log.Error("Image File Path is required.")
			return
		}

		unmountActionBtn.Configure(State("disabled"))
		go func() {
			defer PostEvent(func() { unmountActionBtn.Configure(State("normal")) }, false)
			log.Infof("Unmounting disk image: %s...", target)
			if err := core.UnmountImage(target); err != nil {
				log.Errorf("Unmount failed: %v", err)
				return
			}
			log.Default().Logf(core.SuccessLevel, "Unmounted image file: %s", target)
		}()
	}))

	// Initial scanning trigger
	refreshAllDisks()

	// 4. Start Event Loop
	App.Wait()
}

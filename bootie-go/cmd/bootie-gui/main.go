package main

import (
	"fmt"
	"io"
	"os"
	"runtime"
	"strings"

	humanize "github.com/dustin/go-humanize"
	. "modernc.org/tk9.0"

	log "github.com/charmbracelet/log"
	"ngobach.com/bootie-go/core"
	"ngobach.com/bootie-go/resources"
)

type logWriter struct {
	txt *TextWidget
}

func (lw *logWriter) Write(p []byte) (n int, err error) {
	msg := string(p)
	PostEvent(func() {
		lw.txt.Insert("end", msg)
		lw.txt.See("end")
	}, false)
	return len(p), nil
}

func main() {
	// 1. Initialise the App Window
	App.WmTitle("Bootie System Utility Dashboard")
	WmGeometry(App, "850x650")

	// Create main container frame
	mainFrame := TFrame()
	Pack(mainFrame, Fill("both"), Expand(true), Padx("10"), Pady("10"))

	// 2. Create the Log Text Panel first
	logFrame := TLabelframe(mainFrame, Txt("System Log Console"))
	Pack(logFrame, Side("bottom"), Fill("both"), Expand(false), Pady("5"))

	logTxt := Text(logFrame, Height(10), Background("#1e1e1e"), Foreground("#d4d4d4"))
	Pack(logTxt, Fill("both"), Expand(true))

	// Configure charmbracelet logger to output to both console and GUI log frame
	lw := &logWriter{txt: logTxt}
	log.SetOutput(io.MultiWriter(os.Stderr, lw))

	// 3. Create the Notebook (Tabs) Container
	nb := TNotebook(mainFrame)
	Pack(nb, Side("top"), Fill("both"), Expand(true))

	// ============================================
	// TAB 1: INITIALIZE DISK
	// ============================================
	initTab := nb.TFrame()
	nb.Add(initTab, Txt("Initialize Disk"))

	initContent := TFrame(initTab)
	Pack(initContent, Fill("both"), Expand(true), Padx("20"), Pady("20"))

	Pack(TLabel(initContent, Txt("Format a raw disk or device, installing UEFI EFI and data partitions."), Font("Helvetica 11 bold")), Anchor("w"), Pady("10"))

	diskSelectRow := TFrame(initContent)
	Pack(diskSelectRow, Fill("x"), Pady("5"))
	Pack(TLabel(diskSelectRow, Txt("Select Target Disk: "), Width(18)), Side("left"), Anchor("w"))
	
	initDiskCombo := TCombobox(diskSelectRow, State("readonly"), Textvariable("initDiskVar"), Width(50))
	Pack(initDiskCombo, Side("left"), Fill("x"), Expand(true), Padx("5"))

	fsRow := TFrame(initContent)
	Pack(fsRow, Fill("x"), Pady("5"))
	Pack(TLabel(fsRow, Txt("Filesystem type: "), Width(18)), Side("left"), Anchor("w"))
	initFsCombo := TCombobox(fsRow, State("readonly"), Values("exfat fat32"), Textvariable("initFsVar"), Width(20))
	Pack(initFsCombo, Side("left"), Padx("5"))
	initFsCombo.Current(0)

	checkRow := TFrame(initContent)
	Pack(checkRow, Fill("x"), Pady("5"))
	noDataVar := Variable(false)
	noDataCheck := TCheckbutton(checkRow, Txt("Skip formatting and extracting data partition (no-data-part)"), noDataVar)
	Pack(noDataCheck, Side("left"), Padx("5"))

	btnRow := TFrame(initContent)
	Pack(btnRow, Fill("x"), Pady("20"))
	initActionBtn := TButton(btnRow, Txt("Initialize Selected Disk"))
	Pack(initActionBtn, Side("left"))

	// ============================================
	// TAB 2: INSTALL BOOT SECTORS
	// ============================================
	installTab := nb.TFrame()
	nb.Add(installTab, Txt("Install Boot"))

	installContent := TFrame(installTab)
	Pack(installContent, Fill("both"), Expand(true), Padx("20"), Pady("20"))

	Pack(TLabel(installContent, Txt("Install bootie boot sectors and MBR directly on a physical disk."), Font("Helvetica 11 bold")), Anchor("w"), Pady("10"))

	installDiskRow := TFrame(installContent)
	Pack(installDiskRow, Fill("x"), Pady("5"))
	Pack(TLabel(installDiskRow, Txt("Select Target Disk: "), Width(18)), Side("left"), Anchor("w"))

	installDiskCombo := TCombobox(installDiskRow, State("readonly"), Textvariable("installDiskVar"), Width(50))
	Pack(installDiskCombo, Side("left"), Fill("x"), Expand(true), Padx("5"))

	installBtnRow := TFrame(installContent)
	Pack(installBtnRow, Fill("x"), Pady("20"))
	installActionBtn := TButton(installBtnRow, Txt("Install Boot Sectors"))
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

	initRefreshBtn := TButton(diskSelectRow, Txt("Refresh List"), Command(refreshAllDisks))
	Pack(initRefreshBtn, Side("right"), Padx("5"))

	installRefreshBtn := TButton(installDiskRow, Txt("Refresh List"), Command(refreshAllDisks))
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

	copierContent := TFrame(copierTab)
	Pack(copierContent, Fill("both"), Expand(true), Padx("20"), Pady("20"))

	Pack(TLabel(copierContent, Txt("Extract embedded resources (EFI / Data assets) directly into any target directory."), Font("Helvetica 11 bold")), Anchor("w"), Pady("10"))

	destRow := TFrame(copierContent)
	Pack(destRow, Fill("x"), Pady("5"))
	Pack(TLabel(destRow, Txt("Target Folder Path: "), Width(18)), Side("left"), Anchor("w"))
	destEntry := TEntry(destRow, Textvariable("destVar"), Width(50))
	Pack(destEntry, Side("left"), Fill("x"), Expand(true), Padx("5"))

	copierButtonsRow := TFrame(copierContent)
	Pack(copierButtonsRow, Fill("x"), Pady("20"))

	copyEfiBtn := TButton(copierButtonsRow, Txt("Extract EFI Files"))
	Pack(copyEfiBtn, Side("left"), Padx("5"))

	copyDataBtn := TButton(copierButtonsRow, Txt("Extract Data Files"))
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

	qemuContent := TFrame(qemuTab)
	Pack(qemuContent, Fill("both"), Expand(true), Padx("20"), Pady("20"))

	Pack(TLabel(qemuContent, Txt("Configure and spin up QEMU emulator testing your setup virtualized."), Font("Helvetica 11 bold")), Anchor("w"), Pady("10"))

	qemuGrid := TFrame(qemuContent)
	Pack(qemuGrid, Fill("x"), Pady("5"))

	makeQemuRow := func(label string, varName string, defaultVal string) *TEntryWidget {
		row := TFrame(qemuGrid)
		Pack(row, Fill("x"), Pady("3"))
		Pack(TLabel(row, Txt(label), Width(18)), Side("left"), Anchor("w"))
		entry := TEntry(row, Textvariable(varName))
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

	qemuArchRow := TFrame(qemuGrid)
	Pack(qemuArchRow, Fill("x"), Pady("3"))
	Pack(TLabel(qemuArchRow, Txt("Target Architecture: "), Width(18)), Side("left"), Anchor("w"))
	
	defaultArch := "x86_64"
	if runtime.GOARCH == "arm64" && runtime.GOOS == "darwin" {
		defaultArch = "arm64"
	}
	qemuArchCombo := TCombobox(qemuArchRow, State("readonly"), Values("x86_64 arm64"), Textvariable("qemuArchVar"), Width(10))
	Pack(qemuArchCombo, Side("left"), Padx("5"))
	if defaultArch == "arm64" {
		qemuArchCombo.Current(1)
	} else {
		qemuArchCombo.Current(0)
	}

	qemuBtnRow := TFrame(qemuContent)
	Pack(qemuBtnRow, Fill("x"), Pady("20"))
	
	qemuActionBtn := TButton(qemuBtnRow, Txt("Launch QEMU Emulator"))
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

	imagesContent := TFrame(imagesTab)
	Pack(imagesContent, Fill("both"), Expand(true), Padx("20"), Pady("20"))

	createFrame := TLabelframe(imagesContent, Txt("Create Raw Disk Image File"))
	Pack(createFrame, Fill("x"), Pady("10"), Padx("5"), Ipady("10"))

	cPathRow := TFrame(createFrame)
	Pack(cPathRow, Fill("x"), Pady("3"), Padx("10"))
	Pack(TLabel(cPathRow, Txt("Output File Path: "), Width(18)), Side("left"), Anchor("w"))
	createPath := TEntry(cPathRow, Textvariable("createPathVar"))
	Pack(createPath, Side("left"), Fill("x"), Expand(true), Padx("5"))

	cSizeRow := TFrame(createFrame)
	Pack(cSizeRow, Fill("x"), Pady("3"), Padx("10"))
	Pack(TLabel(cSizeRow, Txt("Image Size: "), Width(18)), Side("left"), Anchor("w"))
	createSize := TEntry(cSizeRow, Textvariable("createSizeVar"), Width(20))
	Pack(createSize, Side("left"), Padx("5"))
	createSize.Configure(Textvariable("200M"))

	createBtnRow := TFrame(createFrame)
	Pack(createBtnRow, Fill("x"), Pady("10"), Padx("10"))
	createActionBtn := TButton(createBtnRow, Txt("Create Empty Disk Image"))
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

	mountFrame := TLabelframe(imagesContent, Txt("Mount / Unmount Disk Image"))
	Pack(mountFrame, Fill("x"), Pady("10"), Padx("5"), Ipady("10"))

	mPathRow := TFrame(mountFrame)
	Pack(mPathRow, Fill("x"), Pady("3"), Padx("10"))
	Pack(TLabel(mPathRow, Txt("Image File Path: "), Width(18)), Side("left"), Anchor("w"))
	mountPath := TEntry(mPathRow, Textvariable("mountPathVar"))
	Pack(mountPath, Side("left"), Fill("x"), Expand(true), Padx("5"))

	mountBtnRow := TFrame(mountFrame)
	Pack(mountBtnRow, Fill("x"), Pady("10"), Padx("10"))
	
	mountActionBtn := TButton(mountBtnRow, Txt("Mount Image"))
	Pack(mountActionBtn, Side("left"), Padx("5"))

	unmountActionBtn := TButton(mountBtnRow, Txt("Unmount Image"))
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

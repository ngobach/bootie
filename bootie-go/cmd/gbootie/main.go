package main

import (
	"flag"
	"fmt"
	"image/png"
	"io"
	"log"
	"os"
	"path/filepath"
	"strings"
	"time"

	charmlog "github.com/charmbracelet/log"
	"github.com/dustin/go-humanize"
	"github.com/egoist/mygo"
	"github.com/egoist/mygo/ui"
	"ngobach.com/bootie-go/core"
)

type DriveItem struct {
	Identifier  string
	Label       string
	SizeStr     string
	BusType     string
	IsRemovable bool
}

type appState struct {
	win *mygo.Window

	// Target device mode: 0 for USB Drive, 1 for Disk Image
	targetModeIndex int
	selectedDiskStr string
	diskOptions     []string
	drives          []DriveItem

	// Image file mode
	imageFilePath string
	imageSizeIdx  int // 0: 1GB, 1: 2GB, 2: 4GB

	// Partition scheme: 0 for Dual (EFI + Data), 1 for Single (FAT32)
	layoutIndex int

	// Filesystem: 0 for exFAT, 1 for FAT32
	fsIndex int

	// Boot options
	optBootloaders bool
	optModules     bool
	optWinPE       bool

	// Theme mode: 0 for Dark, 1 for Light
	themeMode int

	// Mock mode for UI previews
	mockMode bool

	// Write / Progress tracking
	isWriting     bool
	progressPct   float64
	statusMessage string
}

func newAppState() *appState {
	a := &appState{
		targetModeIndex: 0,
		imageFilePath:   "/tmp/rescue-disk.img",
		imageSizeIdx:    1, // 2 GB
		layoutIndex:     0, // Dual
		fsIndex:         0, // exFAT
		optBootloaders:  true,
		optModules:      true,
		optWinPE:        true,
		themeMode:       1, // Light
	}
	a.scanDisks()
	return a
}

func (a *appState) scanDisks() {
	a.drives = nil
	a.diskOptions = nil

	if !a.mockMode && core.DefaultDiskManager != nil {
		realDisks, err := core.DefaultDiskManager.ScanDisks()
		if err == nil && len(realDisks) > 0 {
			for _, d := range realDisks {
				if d.IsRemovable {
					item := DriveItem{
						Identifier:  d.Identifier,
						Label:       d.Label,
						SizeStr:     humanize.IBytes(uint64(d.Size)),
						BusType:     "USB",
						IsRemovable: true,
					}
					a.drives = append(a.drives, item)
					a.diskOptions = append(a.diskOptions, item.Identifier)
				}
			}
		}
	}

	if a.mockMode && len(a.drives) == 0 {
		a.drives = []DriveItem{
			{Identifier: "/dev/disk4", Label: "SanDisk Ultra Fit USB 3.1", SizeStr: "32.0 GB", BusType: "USB", IsRemovable: true},
			{Identifier: "/dev/disk5", Label: "Kingston DataTraveler Max", SizeStr: "64.0 GB", BusType: "USB-C", IsRemovable: true},
			{Identifier: "/dev/disk6", Label: "Samsung Portable SSD T7", SizeStr: "500.0 GB", BusType: "External SSD", IsRemovable: false},
		}
		for _, d := range a.drives {
			a.diskOptions = append(a.diskOptions, d.Identifier)
		}
	}

	if len(a.drives) == 0 {
		a.diskOptions = []string{"(No USB drives detected)"}
		a.selectedDiskStr = "(No USB drives detected)"
	} else {
		found := false
		for _, opt := range a.diskOptions {
			if opt == a.selectedDiskStr {
				found = true
				break
			}
		}
		if !found {
			a.selectedDiskStr = a.diskOptions[0]
		}
	}
}

func (a *appState) selectedDrive() *DriveItem {
	for i := range a.drives {
		if a.drives[i].Identifier == a.selectedDiskStr {
			return &a.drives[i]
		}
	}
	return nil
}

func (a *appState) startInitialization() {
	if a.isWriting {
		return
	}

	var target string
	var isFile bool
	var layout string
	var fsType string

	if a.layoutIndex == 0 {
		layout = "separate"
		if a.fsIndex == 0 {
			fsType = "exfat"
		} else {
			fsType = "fat32"
		}
	} else {
		layout = "combined"
		fsType = "fat32"
	}

	if a.targetModeIndex == 0 {
		d := a.selectedDrive()
		if d == nil || d.Identifier == "" || d.Identifier == "(No USB drives detected)" {
			go func() {
				mygo.Dialog.Error("No Drive Selected", "Please connect a USB storage drive and click Rescan.")
			}()
			return
		}
		target = d.Identifier
		isFile = false

		if a.mockMode {
			a.startSimulation()
			return
		}

		go func() {
			res, err := mygo.Dialog.Message(mygo.MessageOptions{
				Title:         "Confirm Disk Formatting",
				Message:       fmt.Sprintf("Format and initialize %s (%s, %s)?", d.Identifier, d.Label, d.SizeStr),
				Detail:        "WARNING: ALL existing partitions and data on this device will be PERMANENTLY ERASED!",
				Buttons:       []string{"Erase & Create", "Cancel"},
				DefaultButton: 1, // Default to Cancel for safety
				CancelButton:  1,
			})
			if err != nil || res.Button != 0 {
				return
			}
			a.runInitWorker(target, layout, fsType, isFile)
		}()
		return
	}

	// Disk Image mode
	target = strings.TrimSpace(a.imageFilePath)
	if target == "" {
		go func() {
			mygo.Dialog.Error("Invalid Path", "Please specify a valid file path for the target disk image.")
		}()
		return
	}
	isFile = true

	if a.mockMode {
		a.startSimulation()
		return
	}

	go func() {
		a.runInitWorker(target, layout, fsType, isFile)
	}()
}

func (a *appState) runInitWorker(target, layout, fsType string, isFile bool) {
	if a.win != nil {
		a.win.Update(func() {
			a.isWriting = true
			a.progressPct = 0.0
			a.statusMessage = "Starting initialization..."
		})
	} else {
		a.isWriting = true
		a.progressPct = 0.0
		a.statusMessage = "Starting initialization..."
	}

	defer func() {
		if a.win != nil {
			a.win.Update(func() {
				a.isWriting = false
			})
		} else {
			a.isWriting = false
		}
	}()

	// If Disk Image mode, create/truncate the image file first
	if isFile {
		sizeStr := []string{"1GB", "2GB", "4GB"}[a.imageSizeIdx]
		if a.win != nil {
			a.win.Update(func() {
				a.progressPct = 0.02
				a.statusMessage = fmt.Sprintf("Creating %s raw disk image at %s...", sizeStr, target)
			})
		}
		if err := core.CreateImg(target, sizeStr); err != nil {
			if a.win != nil {
				a.win.Update(func() {
					a.statusMessage = fmt.Sprintf("Failed: %v", err)
				})
			}
			mygo.Dialog.Error("Image Creation Failed", err.Error())
			return
		}
	} else {
		// Physical drive: unmount / lock first
		if a.win != nil {
			a.win.Update(func() {
				a.progressPct = 0.02
				a.statusMessage = fmt.Sprintf("Unmounting and locking %s...", target)
			})
		}
		if err := core.DefaultDiskManager.LockDisk(target); err != nil {
			log.Printf("Warning locking disk: %v", err)
		}
	}

	// Run InitializeDiskWithOptions
	err := core.InitializeDiskWithOptions(core.InitOptions{
		Target:     target,
		Layout:     layout,
		FsType:     fsType,
		NoDataCopy: false,
		OnProgress: func(pct float64, msg string) {
			if a.win != nil {
				a.win.Update(func() {
					a.progressPct = pct
					a.statusMessage = msg
				})
			} else {
				a.progressPct = pct
				a.statusMessage = msg
			}
		},
	})

	if err != nil {
		errMsg := err.Error()
		if strings.Contains(strings.ToLower(errMsg), "permission denied") ||
			strings.Contains(strings.ToLower(errMsg), "operation not permitted") {
			errMsg += "\n\nTip: Formatting physical disks requires root privileges. Please run gbootie with sudo (e.g. 'sudo ./gbootie')."
		}
		if a.win != nil {
			a.win.Update(func() {
				a.statusMessage = fmt.Sprintf("Initialization error: %v", err)
			})
		}
		mygo.Dialog.Error("Initialization Failed", errMsg)
		return
	}

	// Success!
	if a.win != nil {
		a.win.Update(func() {
			a.progressPct = 1.0
			a.statusMessage = "Rescue disk created successfully! Ready to boot."
		})
	}
	mygo.Dialog.Message(mygo.MessageOptions{
		Title:   "Success",
		Message: "Bootie Rescue Disk created successfully!",
		Detail:  fmt.Sprintf("Target: %s\nLayout: %s\nFilesystem: %s", target, layout, strings.ToUpper(fsType)),
		Buttons: []string{"OK"},
	})
}

func (a *appState) startSimulation() {
	if a.isWriting {
		return
	}
	a.isWriting = true
	a.progressPct = 0.0
	a.statusMessage = "Wiping partition table & creating GPT..."

	go func() {
		steps := []struct {
			pct float64
			msg string
		}{
			{0.20, "Writing protective MBR & EFI System Partition (FAT32)..."},
			{0.45, "Formatting payload partition with " + []string{"exFAT", "FAT32"}[a.fsIndex] + "..."},
			{0.70, "Installing GRUB4DOS (BIOS) & GRUB4EFI (UEFI) loaders..."},
			{0.90, "Extracting bare-metal rescue modules & boot menu..."},
			{1.00, "Disk successfully initialized! Ready to boot."},
		}

		for _, s := range steps {
			time.Sleep(600 * time.Millisecond)
			if a.win != nil {
				a.win.Update(func() {
					a.progressPct = s.pct
					a.statusMessage = s.msg
				})
			} else {
				a.progressPct = s.pct
				a.statusMessage = s.msg
			}
		}

		time.Sleep(1 * time.Second)
		if a.win != nil {
			a.win.Update(func() {
				a.isWriting = false
			})
		} else {
			a.isWriting = false
		}
	}()
}

// view renders the simplified, compact native interface
func (a *appState) view(c *ui.Context) {
	if a.themeMode == 0 {
		c.SetTheme(ui.DarkTheme())
	} else {
		c.SetTheme(ui.LightTheme())
	}
	t := c.Theme()

	ui.Column(c).Fill().Padding(16, 18).Gap(10).Background(t.Background).Children(func() {
		// -------------------------------------------------------------
		// Card 1: Select target
		// -------------------------------------------------------------
		ui.Column(c).Padding(10, 14).Gap(8).Radius(8).Background(t.Surface).Border(1, t.Border).Children(func() {
			ui.Row(c).AlignItems(ui.Center).Children(func() {
				ui.Text(c, "Select target").FontSize(13).Bold()
				ui.Spacer(c)
				ui.Segmented(c, &a.targetModeIndex, "USB Drive", "Disk Image").Disabled(a.isWriting)
			})

			if a.targetModeIndex == 0 {
				ui.Row(c).Gap(8).AlignItems(ui.Center).Children(func() {
					ui.Select(c, &a.selectedDiskStr, a.diskOptions).Grow(1).Disabled(a.isWriting || len(a.drives) == 0)
					if ui.Button(c, "Rescan").Disabled(a.isWriting).Clicked() {
						a.scanDisks()
					}
				})
			} else {
				ui.Row(c).Gap(8).AlignItems(ui.Center).Children(func() {
					ui.TextInput(c, &a.imageFilePath).Placeholder("/tmp/rescue-disk.img").Grow(1).Disabled(a.isWriting)
					if ui.Button(c, "Browse...").Disabled(a.isWriting).Clicked() {
						go func() {
							path, err := mygo.Dialog.Save(mygo.SaveDialogOptions{
								Title:       "Choose Target Disk Image File",
								DefaultPath: a.imageFilePath,
								Filters:     []mygo.FileFilter{{Name: "Raw Disk Image", Extensions: []string{"img", "raw"}}},
							})
							if err == nil && path != "" {
								if a.win != nil {
									a.win.Update(func() { a.imageFilePath = path })
								} else {
									a.imageFilePath = path
								}
							}
						}()
					}
					ui.Segmented(c, &a.imageSizeIdx, "1 GB", "2 GB", "4 GB").Disabled(a.isWriting)
				})
			}
		})

		// -------------------------------------------------------------
		// Card 2: Partition Scheme & File System (Compact side-by-side)
		// -------------------------------------------------------------
		ui.Row(c).Gap(10).AlignItems(ui.Stretch).Children(func() {
			ui.Column(c).Grow(1).Padding(10, 14).Gap(6).Radius(8).Background(t.Surface).Border(1, t.Border).Children(func() {
				ui.Text(c, "Partition Scheme").FontSize(13).Bold()
				ui.Segmented(c, &a.layoutIndex, "Dual", "Single").AlignSelf(ui.Start).Disabled(a.isWriting)
			})

			ui.Column(c).Grow(1).Padding(10, 14).Gap(6).Radius(8).Background(t.Surface).Border(1, t.Border).Children(func() {
				ui.Text(c, "File System").FontSize(13).Bold()
				if a.layoutIndex == 1 {
					fsChoice := 1
					ui.Segmented(c, &fsChoice, "exFAT", "FAT32").Disabled(true).AlignSelf(ui.Start)
				} else {
					ui.Segmented(c, &a.fsIndex, "exFAT", "FAT32").Disabled(a.isWriting).AlignSelf(ui.Start)
				}
			})
		})

		// -------------------------------------------------------------
		// Card 3: Target Details & Layout Preview (Equal items a, b, c, d)
		// -------------------------------------------------------------
		ui.Column(c).Padding(10, 14).Gap(8).Radius(8).Background(t.Surface).Border(1, t.Border).Children(func() {
			var destVal, devNameVal, sizeVal string
			if a.targetModeIndex == 0 {
				d := a.selectedDrive()
				if d != nil && d.Identifier != "(No USB drives detected)" {
					destVal = d.Identifier
					devNameVal = d.Label
					sizeVal = d.SizeStr
				} else {
					destVal = "None"
					devNameVal = "(Insert a USB drive & click Rescan)"
					sizeVal = "--"
				}
			} else {
				destVal = a.imageFilePath
				devNameVal = "Disk image"
				sizeVal = []string{"1 GB", "2 GB", "4 GB"}[a.imageSizeIdx]
			}

			// a. Destination
			ui.Column(c).Gap(2).Children(func() {
				ui.Text(c, "Destination").FontSize(13).Bold().TextColor(t.Text)
				ui.Text(c, destVal).FontSize(11).TextColor(t.TextMuted)
			})

			// b. Device name
			ui.Column(c).Gap(2).Children(func() {
				ui.Text(c, "Device name").FontSize(13).Bold().TextColor(t.Text)
				ui.Text(c, devNameVal).FontSize(11).TextColor(t.TextMuted)
			})

			// c. Size
			ui.Column(c).Gap(2).Children(func() {
				ui.Text(c, "Size").FontSize(13).Bold().TextColor(t.Text)
				ui.Text(c, sizeVal).FontSize(11).TextColor(t.TextMuted)
			})

			// d. Layout Preview
			ui.Column(c).Gap(5).Children(func() {
				ui.Text(c, "Layout Preview").FontSize(13).Bold().TextColor(t.Text)
				ui.Row(c).Height(26).Radius(5).Border(1, t.Border).Clip().AlignItems(ui.Stretch).Children(func() {
					ui.Column(c).Width(48).FillHeight().Center().Background(ui.Hex("#334155")).Children(func() {
						ui.Text(c, "MBR").FontSize(10).Bold().TextColor(ui.Hex("#94a3b8"))
					})
					if a.layoutIndex == 0 {
						ui.Column(c).Width(150).FillHeight().Center().Background(ui.Hex("#0284c7")).Children(func() {
							ui.Text(c, "EFI (FAT32) 100 MB").FontSize(10).Bold().TextColor(ui.Hex("#ffffff"))
						})
						dataFsName := []string{"exFAT", "FAT32"}[a.fsIndex]
						ui.Column(c).Grow(1).FillHeight().Center().Background(ui.Hex("#059669")).Children(func() {
							ui.Text(c, fmt.Sprintf("Rescue Data (%s)", dataFsName)).FontSize(10).Bold().TextColor(ui.Hex("#ffffff"))
						})
					} else {
						ui.Column(c).Grow(1).FillHeight().Center().Background(ui.Hex("#0d9488")).Children(func() {
							ui.Text(c, "Boot & Rescue Data (FAT32)").FontSize(10).Bold().TextColor(ui.Hex("#ffffff"))
						})
					}
				})
			})
		})

		// -------------------------------------------------------------
		// Card 4: Action & Execution
		// -------------------------------------------------------------
		ui.Row(c).AlignItems(ui.Center).Gap(14).Children(func() {
			btnText := "Create Rescue Disk"
			if a.isWriting {
				btnText = "Writing..."
			}
			canStart := !a.isWriting && (a.targetModeIndex == 1 || (a.selectedDrive() != nil && a.selectedDrive().Identifier != "" && a.selectedDrive().Identifier != "(No USB drives detected)"))
			btn := ui.PrimaryButton(c, btnText).Disabled(!canStart)
			if btn.Clicked() && canStart {
				a.startInitialization()
			}

			if a.isWriting {
				ui.Progress(c, a.progressPct).Grow(1)
				ui.Textf(c, "%.0f%%", a.progressPct*100).FontSize(12).Bold()
			}
		})

		if a.statusMessage != "" {
			ui.Row(c).Gap(6).AlignItems(ui.Center).Children(func() {
				ui.Text(c, "●").FontSize(9).TextColor(t.Accent)
				ui.Text(c, a.statusMessage).FontSize(11).TextColor(t.Accent)
			})
		}
	})
}

func main() {
	pngPath := flag.String("png", "", "render one frame to PNG and exit")
	darkFlag := flag.Bool("dark", false, "start in dark mode")
	modeFlag := flag.Int("mode", -1, "target mode: 0 for USB, 1 for Disk Image")
	sizeFlag := flag.Int("size", -1, "image size index: 0 for 1GB, 1 for 2GB, 2 for 4GB")
	layoutFlag := flag.Int("layout", -1, "partition layout index: 0 for Dual, 1 for Single")
	mockFlag := flag.Bool("mock", false, "use mock USB drives for demonstration")
	flag.Parse()

	// Initialize log file in temp directory
	tempDir := os.TempDir()
	timestamp := time.Now().Format("20060102-150405")
	logFilename := filepath.Join(tempDir, fmt.Sprintf("gbootie-%s.log", timestamp))
	if logFile, err := os.Create(logFilename); err == nil {
		defer logFile.Close()
		charmlog.SetOutput(io.MultiWriter(os.Stderr, logFile))
	}

	app := newAppState()
	if *mockFlag {
		app.mockMode = true
		app.scanDisks()
	}
	if *darkFlag {
		app.themeMode = 0
	} else {
		app.themeMode = 1
	}
	if *modeFlag >= 0 {
		app.targetModeIndex = *modeFlag
	}
	if *sizeFlag >= 0 {
		app.imageSizeIdx = *sizeFlag
	}
	if *layoutFlag >= 0 {
		app.layoutIndex = *layoutFlag
	}

	// Ultra-compact window size: 600 x 432
	winW, winH := 600, 432

	if *pngPath != "" {
		img := ui.Render(app.view, winW, winH, 2)
		f, err := os.Create(*pngPath)
		if err != nil {
			log.Fatalf("failed to create PNG file: %v", err)
		}
		defer f.Close()
		if err := png.Encode(f, img); err != nil {
			log.Fatalf("failed to encode PNG: %v", err)
		}
		fmt.Printf("Rendered compact MyGo mockup frame to %s\n", *pngPath)
		return
	}

	mygo.App.WhenReady(func() {
		app.win = mygo.NewWindow(mygo.WindowOptions{
			Title:          "Bootie",
			Width:          winW,
			Height:         winH,
			MinWidth:       540,
			MinHeight:      winH,
			UseContentSize: true,
			Content:        ui.View(app.view),
		})
	})

	if err := mygo.App.Run(); err != nil {
		log.Fatal(err)
	}
}

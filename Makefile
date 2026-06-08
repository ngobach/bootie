GOOS_DARWIN  := darwin
GOOS_LINUX   := linux
GOOS_WINDOWS := windows

GOARCH       := amd64

LDFLAGS      := -ldflags="-s -w"
BUILD_DIR    := build
BOOTIE_GO_DIR := bootie-go

# CLI executables
CLI_DARWIN  := $(BUILD_DIR)/bootie-darwin
CLI_LINUX   := $(BUILD_DIR)/bootie-linux
CLI_WINDOWS := $(BUILD_DIR)/bootie-windows.exe

# GUI executables
GUI_DARWIN  := $(BUILD_DIR)/bootie-gui-darwin
GUI_LINUX   := $(BUILD_DIR)/bootie-gui-linux
GUI_WINDOWS := $(BUILD_DIR)/bootie-gui-windows.exe

# Default target — build all platform binaries
default: all

# Bootie-ext modules
MOD_DEST := $(BOOTIE_GO_DIR)/resources/data-part/mod

# Build and copy bootie-ext modules
mod:
	$(MAKE) -C bootie-ext
	rm -rf $(MOD_DEST)
	cp -r bootie-ext/mod $(MOD_DEST)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(MOD_DEST)

# Prepare build directory
build: mod
	mkdir -p $(BUILD_DIR)

# Build macOS (Darwin) executables
$(CLI_DARWIN): build
	cd $(BOOTIE_GO_DIR) && GOOS=$(GOOS_DARWIN) GOARCH=$(GOARCH) go build -trimpath $(LDFLAGS) -o ../$(CLI_DARWIN) ./cmd/bootie

$(GUI_DARWIN): build
	cd $(BOOTIE_GO_DIR) && GOOS=$(GOOS_DARWIN) GOARCH=$(GOARCH) go build -trimpath $(LDFLAGS) -o ../$(GUI_DARWIN) ./cmd/bootie-gui

# Build Linux executables
$(CLI_LINUX): build
	cd $(BOOTIE_GO_DIR) && GOOS=$(GOOS_LINUX) GOARCH=$(GOARCH) go build -trimpath $(LDFLAGS) -o ../$(CLI_LINUX) ./cmd/bootie

$(GUI_LINUX): build
	cd $(BOOTIE_GO_DIR) && GOOS=$(GOOS_LINUX) GOARCH=$(GOARCH) go build -trimpath $(LDFLAGS) -o ../$(GUI_LINUX) ./cmd/bootie-gui

# Build Windows executables
$(CLI_WINDOWS): build
	cd $(BOOTIE_GO_DIR) && GOOS=$(GOOS_WINDOWS) GOARCH=$(GOARCH) go build -trimpath $(LDFLAGS) -o ../$(CLI_WINDOWS) ./cmd/bootie

$(GUI_WINDOWS): build
	cd $(BOOTIE_GO_DIR) && GOOS=$(GOOS_WINDOWS) GOARCH=$(GOARCH) go build -trimpath $(LDFLAGS) -o ../$(GUI_WINDOWS) ./cmd/bootie-gui

# Aggregate target — build all platform executables for both CLI and GUI
all: $(CLI_DARWIN) $(GUI_DARWIN) $(CLI_LINUX) $(GUI_LINUX) $(CLI_WINDOWS) $(GUI_WINDOWS)

# Declare phony targets to avoid conflicts with same-named files
.PHONY: default clean build all mod

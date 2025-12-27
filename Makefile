GOOS_DARWIN  := darwin
GOOS_LINUX   := linux
GOOS_WINDOWS := windows

GOARCH       := amd64

BUILD_DIR    := build
BOOTIE_GO_DIR := bootie-go

EXECUTABLE_DARWIN  := $(BUILD_DIR)/bootie-darwin
EXECUTABLE_LINUX   := $(BUILD_DIR)/bootie-linux
EXECUTABLE_WINDOWS := $(BUILD_DIR)/bootie-windows.exe

# Default target — build all platform binaries
default: all

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Prepare build directory
build:
	mkdir -p $(BUILD_DIR)

# Build macOS (Darwin) executable
$(EXECUTABLE_DARWIN): build
	cd $(BOOTIE_GO_DIR) && GOOS=$(GOOS_DARWIN) go build -o ../$(EXECUTABLE_DARWIN) .

# Build Linux executable
$(EXECUTABLE_LINUX): build
	cd $(BOOTIE_GO_DIR) && GOOS=$(GOOS_LINUX) GOARCH=$(GOARCH) go build -o ../$(EXECUTABLE_LINUX) .

# Build Windows executable
$(EXECUTABLE_WINDOWS): build
	cd $(BOOTIE_GO_DIR) && GOOS=$(GOOS_WINDOWS) GOARCH=$(GOARCH) go build -o ../$(EXECUTABLE_WINDOWS) .

# Aggregate target — build all platform executables
all: $(EXECUTABLE_DARWIN) $(EXECUTABLE_LINUX) $(EXECUTABLE_WINDOWS)

# Declare phony targets to avoid conflicts with same-named files
.PHONY: default clean build all

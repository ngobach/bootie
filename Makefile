GOOS_DARWIN  := darwin
GOOS_LINUX   := linux
GOOS_WINDOWS := windows

GOARCH       := amd64

BUILD_DIR    := build
BOOTIE_GO_DIR := bootie-go

EXECUTABLE_DARWIN  := $(BUILD_DIR)/bootie-darwin
EXECUTABLE_LINUX   := $(BUILD_DIR)/bootie-linux
EXECUTABLE_WINDOWS := $(BUILD_DIR)/bootie-windows.exe

default: all

clean:
	rm -rf $(BUILD_DIR)

build:
	mkdir -p $(BUILD_DIR)

$(EXECUTABLE_DARWIN): build
	cd $(BOOTIE_GO_DIR) && GOOS=$(GOOS_DARWIN) go build -o ../$(EXECUTABLE_DARWIN) .

$(EXECUTABLE_LINUX): build
	cd $(BOOTIE_GO_DIR) && GOOS=$(GOOS_LINUX) GOARCH=$(GOARCH) go build -o ../$(EXECUTABLE_LINUX) .

$(EXECUTABLE_WINDOWS): build
	cd $(BOOTIE_GO_DIR) && GOOS=$(GOOS_WINDOWS) GOARCH=$(GOARCH) go build -o ../$(EXECUTABLE_WINDOWS) .

all: $(EXECUTABLE_DARWIN) $(EXECUTABLE_LINUX) $(EXECUTABLE_WINDOWS)

.PHONY: default clean build all

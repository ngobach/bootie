SRC_WIM       := assets/seed/boot.wim
SRC_OVERLAY   := src/wim-overlay

BUILD_DIR     := build
BUILD_OVERLAY := build/overlay
BUILD_WIM     := build/wim/winpe.wim

NTLOADER_URL  := https://github.com/grub4dos/ntloader/releases/download/v3.0.7/ntloader.zip
ULUA_URL      := https://ulua.io/download/ulua~latest.zip
WINXSHELL_ZIP := assets/winxshell/WinXShell.zip

WIM_BINS     := wimappend wiminfo
WIM_PATHS    := $(foreach bin,$(WIM_BINS),$(shell command -v $(bin) 2>/dev/null))

.PHONY: all
all: check-deps build

.PHONY: $(BUILD_OVERLAY)

.PHONY: check-deps
check-deps:
	@missing=""; \
	for bin in $(WIM_BINS); do \
		path=$$(command -v $$bin 2>/dev/null); \
		if [ -z "$$path" ]; then \
			missing="$$missing $$bin"; \
		else \
			echo "  found: $$path"; \
		fi \
	done; \
	if [ -n "$$missing" ]; then \
		echo "ERROR: missing wimlib binaries:$$missing"; \
		echo "Install wimlib (e.g. brew install wimlib)"; \
		exit 1; \
	fi; \
	wimlib-imagex --version

$(BUILD_OVERLAY): $(SRC_OVERLAY) build/.ulua.stamp build/.winxshell.stamp
	mkdir -p $(BUILD_OVERLAY)
	cp -r $(SRC_OVERLAY)/. $(BUILD_OVERLAY)/
	cp -r build/ulua $(BUILD_OVERLAY)/
	cp -r build/winxshell/. $(BUILD_OVERLAY)/WinXShell/
	mkdir -p $(BUILD_OVERLAY)/Windows/System32
	cp ./assets/pecmd/PECMD.EXE $(BUILD_OVERLAY)/Windows/System32/PECMD.EXE

$(BUILD_WIM): check-deps $(BUILD_OVERLAY)
	@if [ ! -f "$(SRC_WIM)" ]; then \
		echo "ERROR: source WIM not found: $(SRC_WIM)"; \
		echo "Place a boot.wim in the wpefw directory and try again."; \
		exit 1; \
	fi
	mkdir -p $(dir $(BUILD_WIM))
	cp $(SRC_WIM) $(BUILD_WIM)
	wimupdate $(BUILD_WIM) 1 --command="add $(BUILD_OVERLAY) /"

.PHONY: build
build: $(BUILD_WIM)

.PHONY: run
run: $(BUILD_WIM) build/.ntloader.stamp
	qemu-system-x86_64 \
		-M q35 \
        -m 2G \
        -smp cores=4,threads=1 \
        -accel tcg,thread=multi,tb-size=1024 \
		-bios ../resources/OVMF.fd \
		-s \
		-debugcon file:/tmp/debugcon.log \
		-drive file=fat:rw:build/wim,format=raw \
		-drive file=fat:rw:assets/grub4dos,format=raw

build/.ntloader.stamp:
	mkdir -p build/ntloader
	curl -L -o build/ntloader/ntloader.zip "$(NTLOADER_URL)"
	unzip -o build/ntloader/ntloader.zip ntloader initrd.cpio -d build/ntloader/
	mkdir -p build/wim
	cp build/ntloader/ntloader build/wim/ntloader
	cp build/ntloader/initrd.cpio build/wim/initrd.cpio
	touch $@

build/.ulua.stamp:
	rm -rf build/ulua
	mkdir -p build
	curl -L -o build/ulua.zip "$(ULUA_URL)"
	unzip -o build/ulua.zip -d build/
	touch $@

build/.winxshell.stamp: $(WINXSHELL_ZIP)
	rm -rf build/winxshell
	mkdir -p build/winxshell
	unzip -o "$(WINXSHELL_ZIP)" -d build/winxshell/
	touch $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

SRC_WIM       := assets/seed/boot.wim
SRC_OVERLAY   := src/wim-overlay

BUILD_DIR     := build
BUILD_OVERLAY := build/overlay
BUILD_WIM     := build/wim/winpe.wim

NTLOADER_URL  := https://github.com/grub4dos/ntloader/releases/download/v3.0.7/ntloader.zip
ULUA_URL      := https://ulua.io/download/ulua~latest.zip
WINXSHELL_7Z := assets/winxshell/WinXShell_rc4.0b218.7z
EXPLORERPP_URL := https://download.explorerplusplus.com/stable/1.4.0/explorerpp_x64.zip
PECMD_7Z       := assets/pecmd/PECMD88.05.62-26-05-15x64-EN2ba2.7z

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

$(BUILD_OVERLAY): $(SRC_OVERLAY) build/.ulua.stamp build/.winxshell.stamp build/.explorerpp.stamp build/.pecmd.stamp
	mkdir -p $(BUILD_OVERLAY)
	cp -r $(SRC_OVERLAY)/. $(BUILD_OVERLAY)/
	cp -r build/ulua $(BUILD_OVERLAY)/
	cp -r build/winxshell/WinXShell_x64/. $(BUILD_OVERLAY)/WinXShell/
	cp assets/winxshell/WinXShell.jcfg $(BUILD_OVERLAY)/WinXShell/WinXShell.jcfg
	mkdir -p "$(BUILD_OVERLAY)/Program Files/Explorer++"
	cp -r build/explorerpp/. "$(BUILD_OVERLAY)/Program Files/Explorer++/"
	mkdir -p $(BUILD_OVERLAY)/Windows/System32
	cp build/pecmd/PECMD.EXE $(BUILD_OVERLAY)/Windows/System32/PECMD.EXE
	cp "$(BUILD_OVERLAY)/Program Files/Explorer++/Explorer++.exe" $(BUILD_OVERLAY)/Windows/explorer.exe

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
        -m 1G \
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

build/.winxshell.stamp: $(WINXSHELL_7Z)
	rm -rf build/winxshell
	mkdir -p build/winxshell
	7z x -y "$(WINXSHELL_7Z)" -obuild/winxshell WinXShell_x64
	touch $@

build/.explorerpp.stamp:
	rm -rf build/explorerpp
	mkdir -p build/explorerpp
	curl -L -o build/explorerpp/explorerpp.zip "$(EXPLORERPP_URL)"
	unzip -o build/explorerpp/explorerpp.zip -d build/explorerpp/
	rm -f build/explorerpp/explorerpp.zip
	touch $@

build/.pecmd.stamp: $(PECMD_7Z)
	rm -rf build/pecmd
	mkdir -p build/pecmd
	7z x -y "$(PECMD_7Z)" -obuild/pecmd PECMD88.05.62-26-05-15x64-EN/X64/PECMD.EXE
	mv build/pecmd/PECMD88.05.62-26-05-15x64-EN/X64/PECMD.EXE build/pecmd/PECMD.EXE
	rm -rf build/pecmd/PECMD88.05.62-26-05-15x64-EN
	touch $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

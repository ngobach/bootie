REGISTRY      ?= zot.ngobach.com
DISKGENIUS_OCI := $(REGISTRY)/bootie/diskgenius:v1
WIM_OUT ?= ../../build/apps/pkgs/diskgenius.wim

.PHONY: all clean
all: $(WIM_OUT)

$(WIM_OUT): autoexec.ini
	rm -rf build
	mkdir -p build $(dir $@)
	oras pull $(DISKGENIUS_OCI) -o build
	unzip -o build/diskgenius.zip -d build/
	rm -f build/diskgenius.zip
	mv build/DiskGenius/* build/
	rm -rf build/DiskGenius
	cp autoexec.ini build/
	wimcapture build $@ --compress=LZX --check
	rm -rf build

verify:
	@echo "=== diskgenius.wim ==="
	wiminfo $(WIM_OUT) 1
	@echo
	wimdir $(WIM_OUT) 1

clean:
	rm -f $(WIM_OUT)

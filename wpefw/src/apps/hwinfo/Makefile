REGISTRY   ?= zot.ngobach.com
HWINFO_OCI := $(REGISTRY)/bootie/hwinfo:v1
WIM_OUT ?= ../../build/apps/pkgs/hwinfo.wim

.PHONY: all clean
all: $(WIM_OUT)

$(WIM_OUT): autoexec.ini
	rm -rf build
	mkdir -p build $(dir $@)
	oras pull $(HWINFO_OCI) -o build
	unzip -oj build/hwinfo.zip 'HWiNFO64.exe' -d build/
	rm -f build/hwinfo.zip
	mv build/HWiNFO64.exe build/HWiNFO.exe
	cp autoexec.ini build/
	wimcapture build $@ --compress=LZX --check
	rm -rf build

verify:
	@echo "=== hwinfo.wim ==="
	wiminfo $(WIM_OUT) 1
	@echo
	wimdir $(WIM_OUT) 1

clean:
	rm -f $(WIM_OUT)

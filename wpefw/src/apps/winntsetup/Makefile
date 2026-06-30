REGISTRY      ?= zot.ngobach.com
WINNTSETUP_OCI := $(REGISTRY)/bootie/winntsetup:v1
WIM_OUT ?= ../../build/apps/pkgs/winntsetup.wim

.PHONY: all clean
all: $(WIM_OUT)

$(WIM_OUT): autoexec.ini
	rm -rf build
	mkdir -p build $(dir $@)
	oras pull $(WINNTSETUP_OCI) -o build
	unzip -o build/winntsetup.zip -d build/
	rm -f build/winntsetup.zip
	cp autoexec.ini build/
	wimcapture build $@ --compress=LZX --check
	rm -rf build

verify:
	@echo "=== winntsetup.wim ==="
	wiminfo $(WIM_OUT) 1
	@echo
	wimdir $(WIM_OUT) 1

clean:
	rm -f $(WIM_OUT)

REGISTRY   ?= zot.ngobach.com
BOOTICE_OCI := $(REGISTRY)/bootie/bootice:v1
WIM_OUT ?= ../../build/apps/pkgs/bootice.wim

.PHONY: all clean
all: $(WIM_OUT)

$(WIM_OUT): autoexec.ini
	rm -rf build
	mkdir -p build $(dir $@)
	oras pull $(BOOTICE_OCI) -o build
	unzip -oj build/bootice.zip 'BOOTICEx64*.exe' -d build/
	for f in build/BOOTICEx64*.exe; do mv "$$f" build/BOOTICE.exe; done
	rm -f build/bootice.zip
	cp autoexec.ini build/
	wimcapture build $@ --compress=LZX --check
	rm -rf build

verify:
	@echo "=== bootice.wim ==="
	wiminfo $(WIM_OUT) 1
	@echo
	wimdir $(WIM_OUT) 1

clean:
	rm -f $(WIM_OUT)

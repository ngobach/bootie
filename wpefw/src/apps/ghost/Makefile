REGISTRY   ?= zot.ngobach.com
GHOST_OCI  := $(REGISTRY)/bootie/ghost:v1
WIM_OUT ?= ../../build/apps/pkgs/ghost.wim

.PHONY: all clean
all: $(WIM_OUT)

$(WIM_OUT): autoexec.ini
	rm -rf build
	mkdir -p build $(dir $@)
	oras pull $(GHOST_OCI) -o build
	mv build/Ghost64.exe build/GHOST.exe
	rm -f build/Ghost32.exe
	cp autoexec.ini build/
	wimcapture build $@ --compress=LZX --check
	rm -rf build

verify:
	@echo "=== ghost.wim ==="
	wiminfo $(WIM_OUT) 1
	@echo
	wimdir $(WIM_OUT) 1

clean:
	rm -f $(WIM_OUT)

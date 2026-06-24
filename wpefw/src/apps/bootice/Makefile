WIM_OUT ?= ../../build/apps/bootice.wim

.PHONY: all clean
all: $(WIM_OUT)

$(WIM_OUT): autoexec.ini bootice.zip
	rm -rf build
	mkdir -p build
	unzip -oj bootice.zip '*/BOOTICEx64*.exe' -d build/
	for f in build/BOOTICEx64*.exe; do mv "$$f" build/BOOTICE.exe; done
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

.PHONY: all clean run-qemu-bios run-qemu-uefi sync-modules

all: scratch.img data-part sync-modules

scratch.img:
	cd ../bootie-go && go run ./cmd/bootie create-img -t ../runner/scratch.img -s 220M
	cd ../bootie-go && go run ./cmd/bootie init -t ../runner/scratch.img --no-data-part

data-part:
	@if [ ! -d data-part ]; then \
		echo "Creating local data-part directory..."; \
		cp -r ../bootie-go/resources/data-part data-part; \
	fi

sync-modules: data-part
	$(MAKE) -C ../bootie-ext -j8
	@if [ -d ../bootie-ext/mod ]; then \
		echo "Syncing compiled modules to data-part/mod..."; \
		mkdir -p data-part/mod; \
		cp -r ../bootie-ext/mod/* data-part/mod/; \
		find data-part/mod -name '*.d' -delete; \
	fi

run-qemu-bios: scratch.img data-part sync-modules
	qemu-system-x86_64 -m 2G -s -debugcon file:/tmp/debugcon.log -drive file=scratch.img,format=raw -drive file=fat:rw:data-part,format=raw

run-qemu-uefi: scratch.img data-part sync-modules
	qemu-system-x86_64 -bios ../resources/OVMF.fd -m 2G -s -debugcon file:/tmp/debugcon.log -drive file=scratch.img,format=raw -drive file=fat:rw:data-part,format=raw

clean:
	rm -rf scratch.img data-part

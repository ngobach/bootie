default: build/release/bootie

clean:
	rm -rf build

build/release:
	mkdir -p build/release
	cd build/release && cmake -DCMAKE_BUILD_TYPE=MinSizeRel -DB_PRODUCTION_MODE=ON ../.. 

build/release/bootie: build/release
	mkdir -p build/release
	cd build/release && $(MAKE) -j 8 bootie

.PHONY: build/release/bootie

default: all

zig-cross:
	git clone https://github.com/mrexodia/zig-cross.git

clean:
	rm -rf build bootie.exe bootie

build/current/bootie:
	cmake -B build/current -DCMAKE_BUILD_TYPE=MinSizeRel -DB_PRODUCTION_MODE=ON --fresh
	cmake --build build/current -- -j 8 bootie

build/win/bootie.exe: zig-cross
	cmake -B build/win --toolchain "../../zig-cross/x86_64-windows-gnu.cmake" -DCMAKE_BUILD_TYPE=MinSizeRel -DB_PRODUCTION_MODE=ON --fresh
	# cmake -B build/win --toolchain ../../mingw.cmake -DCMAKE_BUILD_TYPE=MinSizeRel -DB_PRODUCTION_MODE=ON --fresh
	cmake --build build/win -- -j 8 bootie

bootie: build/current/bootie
	cp build/current/bootie bootie

bootie.exe: build/win/bootie.exe
	cp build/win/bootie.exe bootie.exe

all: bootie bootie.exe

.PHONY: build/current/bootie bootie build/win/bootie.exe bootie.exe

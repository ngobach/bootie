default: bootie

configure:
	xmake f -p mingw -a x64 -m release -y # --toolchain=zig

bootie: configure
	xmake -v bootie

deploy: bootie
	scp build/mingw/x64/release/bootie.exe windows7:~/bootie.exe

.PHONY: configure make test

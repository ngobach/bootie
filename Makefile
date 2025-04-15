default: all

clean:
	rm -rf bootie.exe bootie.macosx build

build/bootie-go:
	mkdir -p build
	cd bootie-go && go build -o ../build .

bootie.exe:
	xmake f -p mingw -a x86_64 -m release -y
	xmake -rv bootie
	cp build/mingw/x86_64/release/bootie.exe bootie.exe

bootie.macosx:
	xmake f -p macosx -m release -y
	xmake -rv bootie
	cp build/macosx/*/release/bootie bootie.macosx

deploy: bootie.exe
	scp bootie.exe windows7:~/bootie.exe

all: bootie.exe bootie.macosx build/bootie-go

.PHONY: clean bootie.exe bootie.linux bootie.macosx bootie-go

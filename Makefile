default: all

clean:
	rm -rf build

build:
	mkdir -p build

build/bootie-darwin:
	cd bootie-go && GOOS=darwin go build -o ../build/bootie-darwin .

build/bootie-windows.exe:
	cd bootie-go && GOOS=windows go build -o ../build/bootie-windows.exe .

all: build/bootie-darwin build/bootie-windows.exe

.PHONY: default clean build build/bootie-darwin build/bootie-windows.exe

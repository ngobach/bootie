default: all

clean:
	rm -rf build

build:
	mkdir -p build

build/bootie-darwin:
	cd bootie-go && GOOS=darwin go build -o ../build/bootie-darwin .

build/bootie-linux:
	cd bootie-go && GOOS=linux GOARCH=amd64 go build -o ../build/bootie-linux .

build/bootie-windows.exe:
	cd bootie-go && GOOS=windows GOARCH=amd64 go build -o ../build/bootie-windows.exe .

all: build/bootie-darwin build/bootie-linux build/bootie-windows.exe

.PHONY: default clean build build/bootie-darwin build/bootie-linux build/bootie-windows.exe

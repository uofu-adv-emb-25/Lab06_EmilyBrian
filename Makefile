.PHONY: all build clean

all: build

build:
	cmake -S . -B build
	cmake --build build -j

load_test:
	cmake --build build
	picotool load -f build/test/mytest.elf
	picotool reboot -f

clean:
	rm -rf build

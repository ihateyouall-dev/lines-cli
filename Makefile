.PHONY: all build clean test

all: build

build: CMakeLists.txt CMakePresets.json
	cmake --workflow debug-clang

test:
	ctest --preset debug-clang

clean:
	rm -rf build

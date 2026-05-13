.PHONY: all build clean test

all: build

build: CMakeLists.txt CMakePresets.json install-deps
	cmake --workflow release-clang

build-debug: CMakeLists.txt CMakePresets.json install-deps-debug
	cmake --workflow debug-clang

install-deps: conanfile.txt
	conan install . --output-folder=build --build=missing

install-deps-debug: conanfile.txt
	conan install . --output-folder=build --build=missing -s build_type=Debug

test:
	ctest --preset debug-clang

clean:
	rm -rf build

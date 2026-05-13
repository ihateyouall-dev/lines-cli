.PHONY: all build clean test

CMAKE_PRESET ?= release-clang

all: build

build: CMakeLists.txt CMakePresets.json install-deps
	cmake --workflow $(CMAKE_PRESET)

install-deps: conanfile.txt
	conan install . --output-folder=build --build=missing \
		-s compiler.cppstd=20

test:
	ctest --preset $(CMAKE_PRESET)

clean:
	rm -rf build

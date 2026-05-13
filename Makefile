.PHONY: all build clean test

CMAKE_PRESET ?= release-clang
CONAN_BUILD_TYPE ?= Release

all: build

build: CMakeLists.txt CMakePresets.json install-deps
	cmake --workflow $(CMAKE_PRESET)

install-deps: conanfile.txt
	conan install . --output-folder=build --build=missing \
		-s compiler.cppstd=20 -s build_type=$(CONAN_BUILD_TYPE)

test:
	ctest --preset $(CMAKE_PRESET)

clean:
	rm -rf build

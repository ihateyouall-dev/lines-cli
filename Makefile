.PHONY: all build clean test

CMAKE_PRESET ?= conan-release
CONAN_BUILD_TYPE ?= Release
BUILD_DIR ?= build

all: build

build: CMakeLists.txt CMakePresets.json CMakeUserPresets.json install-deps
	cmake --preset $(CMAKE_PRESET)
	cmake --build --preset $(CMAKE_PRESET)

install-deps: conanfile.txt
	conan install . --output-folder=build --build=missing \
		-s compiler.cppstd=20 -s build_type=$(CONAN_BUILD_TYPE)

test:
	ctest --preset $(CMAKE_PRESET)

install:
	cmake --install $(BUILD_DIR)

clean:
	rm -rf build

.PHONY: all build clean test

CMAKE_PRESET ?= release
CMAKE_CONFIGURE_PRESET ?= $(CMAKE_PRESET)
CONAN_BUILD_TYPE ?= Release
BUILD_DIR ?= build
BUILD_TYPE ?= Release

.ONESHELL:

ifeq ($(BUILD_TYPE),Debug)
CMAKE_PRESET := debug
CONAN_BUILD_TYPE := Debug
else ifeq ($(BUILD_TYPE),Test)
CMAKE_PRESET := test
CONAN_BUILD_TYPE := Debug
else ifeq ($(BUILD_TYPE),Coverage)
CMAKE_PRESET := coverage
CONAN_BUILD_TYPE := Debug
endif

# On Windows, Conan generates 'conan-default' configure preset instead of 'conan-release'
ifeq ($(OS),Windows_NT)
ifeq ($(CMAKE_CONFIGURE_PRESET),conan-release)
CMAKE_CONFIGURE_PRESET := conan-default
endif
endif

all: build

configure: CMakeLists.txt CMakePresets.json install-deps
	cmake --preset $(CMAKE_CONFIGURE_PRESET)

build: configure
	cmake --build --preset $(CMAKE_PRESET)

install-deps: conanfile.txt $(HOME)/.conan2/profiles/default
	conan install . --output-folder=$(BUILD_DIR) --build=missing \
		-s compiler.cppstd=20 -s build_type=$(CONAN_BUILD_TYPE)

$(HOME)/.conan2/profiles/default:
	conan profile detect --force

test:
	ctest --preset test

install:
	cmake --install $(BUILD_DIR)

package: build
	cd $(BUILD_DIR)
	cpack

clean:
	rm -rf $(BUILD_DIR)

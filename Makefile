.PHONY: all build clean test

CONAN_BUILD_TYPE ?= Release
CONAN_CXX ?= clang
CONAN_LIBCXX ?= libc++
CMAKE_PRESET ?= release-clang

all: build

build: CMakeLists.txt CMakePresets.json install-deps
	cmake --workflow $(CMAKE_PRESET)

ifeq ($(CONAN_CXX),msvc)
CONAN_LIBCXX_ARG :=
else
CONAN_LIBCXX_ARG := -s compiler.libcxx=$(CONAN_LIBCXX)
endif

install-deps: conanfile.txt
	conan install . --output-folder=build --build=missing \
		-s build_type=$(CONAN_BUILD_TYPE) -s compiler.cppstd=20 \
		-s compiler=$(CONAN_CXX) $(CONAN_LIBCXX_ARG)

test:
	ctest --preset $(CMAKE_PRESET)

clean:
	rm -rf build

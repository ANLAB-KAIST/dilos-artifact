.PHONY: module

APPS = build/compress build/decompress
include ../build_type.mk

module: $(APPS)

build:
	mkdir -p build

build/build.ninja: build
	cd build; cmake -GNinja -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} ..

.PHONY: build-all
build-all: build/build.ninja
	cd build; ninja


.PHONY: $(APPS)
$(APPS): build-all


.PHONY: clean
clean:
	rm -rf build
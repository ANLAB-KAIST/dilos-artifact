.PHONY: module

APPS = build/compress build/decompress

module: $(APPS)

build:
	mkdir -p build

build/build.ninja: build
	cd build; cmake -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo ..

.PHONY: build-all
build-all: build/build.ninja
	cd build; ninja


.PHONY: $(APPS)
$(APPS): build-all


.PHONY: clean
clean:
	rm -rf build
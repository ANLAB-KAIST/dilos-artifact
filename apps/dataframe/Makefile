.PHONY: module
module: build/bin/main

include ../build_type.mk

.PHONY: build/bin/main
build/bin/main:
	mkdir -p build
	cd build; cmake -GNinja -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_CXX_COMPILER=g++-9 ../original
	cd build; ninja bin/main


.PHONY: clean
clean:
	rm -rf build
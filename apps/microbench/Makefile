.PHONY: module

include ../build_type.mk

module:
	@mkdir -p build
	@cd build; cmake -G Ninja -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} ..; ninja

clean:
	rm -rf build usr.manifest

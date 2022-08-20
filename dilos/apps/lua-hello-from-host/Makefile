.PHONY: module clean

SRC = $(shell readlink -f ../..)

lua_path := $(shell which lua)
ifndef lua_path
	lua_path := $(shell which lua5.3)
endif
lua_full_path = $(shell realpath $(lua_path))

module:
	echo "/lua: ${lua_full_path}" > usr.manifest
	echo '/hello.lua: $${MODULE_DIR}/hello.lua' >> usr.manifest
	$(SRC)/scripts/manifest_from_host.sh $(lua_full_path) >> usr.manifest

clean:
	rm -rf usr.manifest

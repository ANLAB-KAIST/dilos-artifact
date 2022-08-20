.PHONY: module clean

SRC = $(shell readlink -f ../..)

lighttpd_path = $(shell realpath $$(which lighttpd)) 

ifneq "$(wildcard /usr/lib64/lighttpd)" ""
lighttpd_lib_path = /usr/lib64/lighttpd
else
lighttpd_lib_path = /usr/lib/lighttpd
endif

module:
	$(SRC)/scripts/manifest_from_host.sh $(lighttpd_path) > usr.manifest
	echo '/www-data/**: $${MODULE_DIR}/www-data/**' >> usr.manifest
	echo '/lighttpd.conf: $${MODULE_DIR}/lighttpd.conf' >> usr.manifest
	@if [ -d $(lighttpd_lib_path) ]; then echo '/usr/lib/lighttpd/**: $(lighttpd_lib_path)/**' >> usr.manifest; else echo 'ERROR: Failed to find lighttpd libraries'; fi

clean:
	rm -rf usr.manifest

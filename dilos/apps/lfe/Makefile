.PHONY: all

LFE_VERSION=0.9.2

all: compile usr.manifest
module: compile usr.manifest

lfe-$(LFE_VERSION).tar.gz:
	wget -O "$@.temp" "https://github.com/rvirding/lfe/archive/v$(LFE_VERSION).tar.gz"
	mv "$@.temp" "$@"

lfe-$(LFE_VERSION): lfe-$(LFE_VERSION).tar.gz
	tar xvf "$<"

compile: lfe-$(LFE_VERSION)
	make -C lfe-$(LFE_VERESION)

usr.manifest: Makefile
	echo '/usr/lib/lfe/ebin/**: $${MODULE_DIR}/lfe-$(LFE_VERSION)/ebin/**' > usr.manifest
	echo '/usr/lib/lfe/src/**: $${MODULE_DIR}/lfe-$(LFE_VERSION)/src/**' >> usr.manifest
	echo '/usr/lib/lfe/include/**: $${MODULE_DIR}/lfe-$(LFE_VERSION)/include/**' >> usr.manifest
	echo '/etc/default/lfe/**: $${MODULE_DIR}/default/**' >> usr.manifest

clean:
	-rm -rf lfe-$(LFE_VERSION)
	-rm -rf lfe-$(LFE_VERSION).tar.gz lfe-$(LFE_VERSION).tar.gz.temp

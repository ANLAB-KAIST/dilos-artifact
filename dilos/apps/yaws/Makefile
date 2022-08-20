.PHONY: all

YAWS_VERSION=1.99

all: install usr.manifest
module: install usr.manifest

yaws-$(YAWS_VERSION).tar.gz:
	wget -O "$@.temp" "http://yaws.hyber.org/download/yaws-$(YAWS_VERSION).tar.gz"
	mv "$@.temp" "$@"

yaws-$(YAWS_VERSION): yaws-$(YAWS_VERSION).tar.gz
	tar xvf "$<"

configure: yaws-$(YAWS_VERSION)
	cd yaws-$(YAWS_VERSION); ./configure --disable-pam

compile: configure
	make -C yaws-$(YAWS_VERSION)

install: compile
	export DESTDIR=$(CURDIR)/ROOTFS; make -C yaws-$(YAWS_VERSION) install
	touch $(CURDIR)/ROOTFS/usr/local/var/log/yaws/.keep

usr.manifest: Makefile
	echo '/usr/lib64/erlang/lib/yaws-$(YAWS_VERSION)/**: $${MODULE_DIR}/ROOTFS/usr/lib64/erlang/lib/yaws-$(YAWS_VERSION)/**' > usr.manifest
	echo '/usr/local/var/yaws/**: $${MODULE_DIR}/ROOTFS/usr/local/var/yaws/**' >> usr.manifest
	echo '/usr/local/var/log/yaws/**: $${MODULE_DIR}/ROOTFS/usr/local/var/log/yaws/**' >> usr.manifest
	echo '/usr/local/etc/yaws/**: $${MODULE_DIR}/ROOTFS/usr/local/etc/yaws/**' >> usr.manifest
	echo '/etc/default/yaws/**: $${MODULE_DIR}/default/**' >> usr.manifest

clean:
	-rm -rf ROOTFS
	-rm -rf yaws-$(YAWS_VERSION)
	-rm -rf yaws-$(YAWS_VERSION).tar.gz yaws-$(YAWS_VERSION).tar.gz.temp

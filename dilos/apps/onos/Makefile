VERSION:=1.2.1
KARAF_VERSION:=3.0.3
NAME:=onos
DIR:=$(NAME)-$(VERSION)
TARBALL:=$(DIR).tar.gz
URL:=http://downloads.onosproject.org/release/$(TARBALL)

module: ROOTFS usr.manifest

$(TARBALL):
	wget -c $(URL)

ROOTFS/$(NAME): $(TARBALL)
	tar xf $(TARBALL)
	mkdir -p ROOTFS
	mv $(DIR) ROOTFS/$(NAME)
	touch ROOTFS/$(NAME)
	find ./assets/etc -maxdepth 1 -type f \
		-exec cp {} ROOTFS/$(NAME)/apache-karaf-$(KARAF_VERSION)/etc/. \;

usr.manifest:
	echo "/$(NAME)/**: \$${MODULE_DIR}/ROOTFS/$(NAME)/**" > usr.manifest

clean:
	rm -rf ROOTFS $(DIR)
	rm -f usr.manifest $(TARBALL)

ROOTFS: ROOTFS/$(NAME)

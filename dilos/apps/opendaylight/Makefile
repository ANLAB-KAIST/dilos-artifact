VERSION := 0.3.0-Lithium
NAME	:= opendaylight
DIR		:= distribution-karaf
TARBALL := $(DIR)-$(VERSION).tar.gz
URL		:= https://nexus.opendaylight.org/content/groups/public/org/opendaylight/integration/$(DIR)/$(VERSION)/$(TARBALL)

module: ROOTFS usr.manifest

$(TARBALL):
	wget -c $(URL)

ROOTFS/$(NAME): $(TARBALL)
	tar xvf $(TARBALL)
	mkdir -p ROOTFS
	mv $(DIR)-$(VERSION) ROOTFS/$(NAME)
	touch ROOTFS/$(NAME)
	find ./assets/etc -type f -exec cp {} ROOTFS/$(NAME)/etc/. \;

ROOTFS: ROOTFS/$(NAME)

usr.manifest:
	echo "/$(NAME)/**: \$${MODULE_DIR}/ROOTFS/$(NAME)/**" > usr.manifest

clean:
	rm -rf distribution-karaf-$(VERSION) ROOTFS

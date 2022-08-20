VERSION:=3.4.6
NAME:=zookeeper
DIR:=$(NAME)-$(VERSION)
TARBALL:=$(DIR).tar.gz
URL:=http://archive.apache.org/dist/$(NAME)/$(NAME)-$(VERSION)/$(TARBALL)


module: ROOTFS usr.manifest

$(TARBALL):
	wget $(URL)

ROOTFS/tmp:
	mkdir -p ROOTFS/tmp/$(NAME)
	touch ROOTFS/tmp

ROOTFS/$(NAME): $(TARBALL)
	tar xvf $(TARBALL)
	mkdir -p ROOTFS
	mv $(DIR) ROOTFS/$(NAME)
	touch ROOTFS/$(NAME)
	find ./assets/conf -type f -exec cp {} ROOTFS/$(NAME)/conf/. \;

usr.manifest:
	echo "/$(NAME)/**: \$${MODULE_DIR}/ROOTFS/$(NAME)/**" > usr.manifest

clean:
	rm -rf ROOTFS $(DIR)
	rm -f usr.manifest $(TARBALL)

ROOTFS: ROOTFS/$(NAME) ROOTFS/tmp

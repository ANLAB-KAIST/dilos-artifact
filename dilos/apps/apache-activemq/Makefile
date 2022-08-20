VERSION:=5.11.1
NAME:=activemq
DIR:=apache-$(NAME)-$(VERSION)
TARBALL:=$(DIR)-bin.tar.gz
URL:=http://archive.apache.org/dist/$(NAME)/$(VERSION)/$(TARBALL)

module: ROOTFS usr.manifest

$(TARBALL):
	wget $(URL)

ROOTFS/$(NAME): $(TARBALL)
	tar xvf $(TARBALL)
	mkdir -p ROOTFS
	mv $(DIR) ROOTFS/$(NAME)
	touch ROOTFS/$(NAME)
	find ./assets/conf -maxdepth 1 -type f -exec cp {} ROOTFS/$(NAME)/conf/. \;
	find ./assets/webapps -maxdepth 1 -exec cp -R {} ROOTFS/$(NAME)/webapps/. \;

usr.manifest:
	echo "/$(NAME)/**: \$${MODULE_DIR}/ROOTFS/$(NAME)/**" > usr.manifest

clean:
	rm -rf ROOTFS $(DIR)
	rm -f usr.manifest $(TARBALL)

ROOTFS: ROOTFS/$(NAME)

VERSION:=7.3.1
NAME:=elasticsearch
DIR:=$(NAME)-$(VERSION)
TARBALL:=elasticsearch-$(VERSION)-linux-x86_64.tar.gz
URL:=https://artifacts.elastic.co/downloads/elasticsearch/$(TARBALL)

module: ROOTFS/$(NAME) ROOTFS/$(NAME)/java/bin/java usr.manifest

$(TARBALL):
	wget -c $(URL)

ROOTFS/$(NAME): $(TARBALL)
	tar xvf $(TARBALL)
	mkdir -p ROOTFS
	mv $(DIR) ROOTFS/$(NAME)
	cd ROOTFS/$(NAME) && rm -rf modules/x-pack-ml && rm -rf modules/ingest-geoip && rm -rf modules/x-pack-security
	touch ROOTFS/$(NAME)
	find ./assets/config -type f -exec cp {} ROOTFS/$(NAME)/config/. \;
	./patch_no_root_check.sh $(VERSION)

usr.manifest:
	echo "/$(NAME)/**: \$${MODULE_DIR}/ROOTFS/$(NAME)/**" > usr.manifest

clean:
	rm -rf ROOTFS $(DIR)
	rm -f usr.manifest $(TARBALL)

ROOTFS/$(NAME)/java/bin/java: ROOTFS/$(NAME)
	ROOTFS/$(NAME)/jdk/bin/jdeps --multi-release 12 --ignore-missing-deps --list-deps ROOTFS/$(NAME)/lib/* | grep -v found | grep -o java.* > /tmp/mods
	ROOTFS/$(NAME)/jdk/bin/jlink --no-header-files --no-man-pages --compress=2 --strip-debug --add-modules $$(echo $$(cat /tmp/mods) | sed 's/ /,/g'),jdk.unsupported,jdk.crypto.cryptoki --output ROOTFS/$(NAME)/java
	rm -rf ROOTFS/$(NAME)/jdk

ROOTFS: ROOTFS/$(NAME)

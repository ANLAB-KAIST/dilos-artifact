HADOOP_VERSION:=2.6
VERSION:=1.4.0
NAME:=spark
DIR:=$(NAME)-$(VERSION)-bin-hadoop$(HADOOP_VERSION)
TARBALL:=$(DIR).tgz
URL:=http://d3kbcqa49mib13.cloudfront.net/$(TARBALL)


module: ROOTFS usr.manifest

$(TARBALL):
	wget -c $(URL)

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

ROOTFS: ROOTFS/$(NAME)

.PHONY: module

VERSION := 10.14.1.0

module: ROOTFS

db-derby-$(VERSION)-bin.tar.gz:
	wget -c http://archive.apache.org/dist/db/derby/db-derby-$(VERSION)/db-derby-$(VERSION)-bin.tar.gz

ROOTFS/db-derby: db-derby-$(VERSION)-bin.tar.gz
	mkdir -p ROOTFS/db-derby
	tar xvf db-derby-$(VERSION)-bin.tar.gz
	cp -a db-derby-$(VERSION)-bin/lib ROOTFS/db-derby

ROOTFS/db-derby/security.policy:
	mkdir -p ROOTFS/db-derby
	cp security.policy ROOTFS/db-derby

ROOTFS: ROOTFS/db-derby ROOTFS/db-derby/security.policy

clean:
	rm -rf ROOTFS db-derby-*

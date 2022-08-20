VERSION=2.7
SOURCE=https://github.com/python/cpython.git

PYTHON_PREFIX=$(PWD)/src/python-install


.PHONY: module clean ROOTFS

SRC=src/cpython

module: usr.manifest

usr.manifest: ROOTFS
	echo '/**: $${MODULE_DIR}/ROOTFS/**' > usr.manifest

clean:
	rm -fr usr.manifest ROOTFS src

# Note: the touch commands below are needed after commands which create files
# "in the past", like wget and tar, and can confuse Make to rebuild a target
# which is new, just pretends to be old.
$(SRC)/.git/HEAD:
	mkdir src
	cd src; git clone $(SOURCE)
	cd $(SRC); git checkout -b $(VERSION) origin/$(VERSION)
	#touch $@

$(SRC)/Makefile: $(SRC)/.git/HEAD
	cd $(SRC); ./configure --enable-shared --prefix=$(PYTHON_PREFIX)
	cd $(SRC); cp Makefile Makefile.orig;
	cd $(SRC); sed -i 's/^LDFLAGS=.*/LDFLAGS=-shared/g' Makefile

$(SRC)/python: $(SRC)/Makefile
	cd $(SRC); $(MAKE) python
	cd $(SRC); mv python python.so
	# Re-link python, this time as ELF executable.
	# This is required for make install to succeed.
	cd $(SRC); $(MAKE) -f Makefile.orig install

ROOTFS: $(SRC)/python
	rm ROOTFS -rf
	mkdir -p ROOTFS/usr/lib

	cp $(SRC)/python.so ROOTFS/python
	cp $(PYTHON_PREFIX)/lib/libpython$(VERSION).so.1.0 ROOTFS/usr/lib
	# Copy the Python environment. Since we'd like to exclude some dirs, rsync is used
	rsync -a $(PYTHON_PREFIX)/lib/python$(VERSION) ROOTFS/lib --exclude test --exclude unittest \
		--exclude '*.pyc' --exclude '*.pyo' --exclude '*.egg-info' --exclude '*.o' --exclude 'libpython'$(VERSION)'.a'
	# TODO maybe .so files in ROOTFS/lib/python2.7/lib-dynload should be stripped.
	./copy-required-system-libraries.sh

.DELETE_ON_ERROR:

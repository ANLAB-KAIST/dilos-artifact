
SOURCE=http://download.lighttpd.net/lighttpd/releases-1.4.x/lighttpd-1.4.54.tar.xz

.PHONY: module clean

SRC=upstream/lighttpd

module: usr.manifest

usr.manifest: $(SRC)/lighttpd.so
	echo '[manifest]' > usr.manifest
	echo '/lighttpd.so: $${MODULE_DIR}/$(SRC)/lighttpd.so' >> usr.manifest
	ldd $(SRC)/lighttpd.so | grep -v 'libc.so\|libpthread.so\|libdl.so\|=>\W*(' |\
		sed -n 's/ *[^ ] *\(.*\) => \(.*\) .*/\/\1: \2/p' \
		>> usr.manifest
	ls upstream/lighttpd/src/.libs/*.so | sed 's/\(.*\)\/mod_\(.*\)/\/lighttpd\/lib\/mod_\2: $${MODULE_DIR}\/upstream\/lighttpd\/src\/.libs\/mod_\2/' >> usr.manifest
	echo '/lighttpd/www-data/**: $${MODULE_DIR}/www-data/**' >> usr.manifest
	echo '/lighttpd/lighttpd.conf: $${MODULE_DIR}/lighttpd.conf' >> usr.manifest

clean:
	rm -fr upstream lib
	rm -f upstream.tar usr.manifest

# Note: the touch commands below are needed after commands which create files
# "in the past", like wget and tar, and can confuse Make to rebuild a target
# which is new, just pretends to be old.
upstream.tar:
	wget -c -O $@ $(SOURCE)
	touch $@

upstream/lighttpd/Makefile.in: upstream.tar
	mkdir upstream
	tar -C upstream -xf upstream.tar
	cd upstream; ln -sf lighttpd-* lighttpd
	touch upstream/lighttpd/Makefile.in

$(SRC)/libtool-orig: $(SRC)/Makefile.in
	cd $(SRC); ./configure --prefix=/lighttpd
	mv $(SRC)/libtool $(SRC)/libtool-orig

$(SRC)/libtool: $(SRC)/libtool-orig
	sed 's/^\(export_dynamic_flag_spec=.*\)"/\1 -shared"/' $(SRC)/libtool-orig > $(SRC)/libtool

$(SRC)/lighttpd.so: $(SRC)/libtool
	make -C $(SRC) CFLAGS="-O2 -fPIC"
	mv $(SRC)/src/lighttpd $(SRC)/lighttpd.so

.DELETE_ON_ERROR:

.PHONY: module version_check test clean
module: usr.manifest

CFLAGS=-fPIC -Wall -shared

RUBY_VER=2.2.4

RUBY_LDFLAGS=-lm -lcrypt -ldl -lpthread -lruby
LIBS=-Lupstream/ruby
INCS=-Iupstream/ruby/include -Iupstream/ruby/.ext/include/x86_64-linux/

LIBRUBY=upstream/ruby/libruby.so

OBSTACK_OBJS=upstream/obstack-1.2/.libs/obstack.o upstream/obstack-1.2/.libs/obstack_printf.o upstream/obstack-1.2/.libs/obstack_vprintf.o
LIBOBSTACK_A=upstream/obstack-1.2/.libs/libobstack.a

version_check:
	@test 1 -eq `ruby --version|grep -e "^ruby ${RUBY_VER}"|wc -l` || (echo "Please install ruby ${RUBY_VER}." && exit 1)

upstream/ruby: version_check
	mkdir -p upstream
	cd upstream && git clone --depth 1 -b v$(subst .,_,${RUBY_VER}) https://github.com/ruby/ruby.git
	cd upstream/ruby && patch -p1 < ../../no-use-sleepy-thread-timer-ruby.patch
	cd upstream/ruby && patch -p1 < ../../no-ipv6.patch
	cd upstream/ruby && patch -p1 < ../../disable-fstat.patch
	cd upstream/ruby && autoconf
	cd upstream/ruby && ./configure --libdir=/usr/lib --enable-pic --enable-shared --disable-install-doc --disable-install-rdoc --disable-ipv6

upstream/obstack-1.2.tar.bz2:
	mkdir -p upstream
	cd upstream && wget -c -O obstack-1.2.tar.bz2 "http://sourceforge.jp/frs/g_redir.php?m=jaist&f=%2Flibfirm%2Fobstack%2F1.2%2Fobstack-1.2.tar.bz2"
	cd upstream && tar xvf obstack-1.2.tar.bz2
	cd upstream/obstack-1.2/ && patch -p1 < ../../obstack_vprintf.patch
	cd upstream/obstack-1.2/ && ./configure

upstream/bundler:
	mkdir -p upstream
	cd upstream && git clone --depth 1 https://github.com/bundler/bundler.git

upstream/cert.pem:
	mkdir -p upstream
	cd upstream && wget -c -O cert.pem "http://curl.haxx.se/ca/cacert.pem"

${LIBOBSTACK_A}: upstream/obstack-1.2.tar.bz2
	cd upstream/obstack-1.2/ && make

${LIBRUBY}: upstream/ruby
	cd upstream/ruby && make

ruby.so: ${LIBOBSTACK_A} ${LIBRUBY}
	cc ${CFLAGS} -c ${INCS} upstream/ruby/main.c 
	cc ${CFLAGS} -o ruby.so ./main.o ${OBSTACK_OBJS} ${LIBS} ${RUBY_LDFLAGS}

usr.manifest: ruby.so upstream/bundler upstream/cert.pem 
	cp -a ./upstream/ruby/.ext/common/* ./upstream/ruby/lib/
	cp -a ./upstream/bundler/lib/* ./upstream/ruby/lib/
	echo '/ruby.so: $${MODULE_DIR}/ruby.so' > usr.manifest
	echo '/**: $${MODULE_DIR}/upstream/ruby/bin/**' >> usr.manifest
	echo '/libruby.so.2.2: $${MODULE_DIR}/upstream/ruby/libruby.so' >> usr.manifest
	echo '/usr/share/ruby/**: $${MODULE_DIR}/upstream/ruby/lib/**' >> usr.manifest
	echo '/usr/lib64/ruby/rbconfig.rb: $${MODULE_DIR}/upstream/ruby/rbconfig.rb' >> usr.manifest
	echo '/usr/lib64/ruby/**: $${MODULE_DIR}/upstream/ruby/.ext/x86_64-linux/**' >> usr.manifest
	echo '/usr/local/ssl/cert.pem: $${MODULE_DIR}/upstream/cert.pem' >> usr.manifest
	find upstream/ruby/.ext/x86_64-linux/ -name '*.so' -exec ldd {} \;|awk '{print $$1,":",$$3}'|grep "/lib"|grep -v "ld-linux"|grep -v "libc.so"|grep -v "libresolv.so.2"|grep -v "libpthread.so"|grep -v "libdl.so"|grep -v "libm.so"|sort|uniq|sed -e "s/ //" \
		>> usr.manifest
	find upstream/ruby/ -name "libruby.so" -exec ldd {} \;|awk '{print $$1,":",$$3}' |grep "/lib"|grep -v "ld-linux"|grep -v "libc.so"|grep -v "libresolv.so.2"|grep -v "libpthread.so"|grep -v "libdl.so"|grep -v "libm.so"|sort|uniq|sed -e "s/ //" \
		>> usr.manifest
	: > bootfs.manifest

ROOTFS: module
	mkdir -p ./ROOTFS/usr/lib/ruby/2.2.0/x86_64-linux
	cp -a ./upstream/ruby/.ext/common/* ./ROOTFS/usr/lib/ruby/2.2.0
	cp -a ./upstream/bundler/lib/* ./ROOTFS/usr/lib/ruby/2.2.0
	cp ruby.so ./ROOTFS/
	cp -a ./upstream/ruby/bin/* ./ROOTFS/
	cp ./upstream/ruby/libruby.so ./ROOTFS/libruby.so.2.2
	cp -a ./upstream/ruby/lib/* ./ROOTFS/usr/lib/ruby/2.2.0
	cp ./upstream/ruby/rbconfig.rb ./ROOTFS/usr/lib/ruby/2.2.0/x86_64-linux/rbconfig.rb
	cp -a ./upstream/ruby/.ext/x86_64-linux/* ./ROOTFS/usr/lib/ruby/2.2.0/x86_64-linux/
	mkdir -p ./ROOTFS/usr/local/ssl
	cp ./upstream/cert.pem ./ROOTFS/usr/local/ssl/cert.pem
	find upstream/ruby/.ext/x86_64-linux/ -name '*.so' -exec ldd {} \;|awk '{print "cp",$$3,"./ROOTFS/"}'|grep "/lib"|grep -v "ld-linux"|grep -v "libc.so"|grep -v "libpthread.so"|grep -v "libdl.so"|grep -v "libm.so"|grep -v "libruby.so"|sort|uniq > tmp.sh
	find upstream/ruby/ -name "libruby.so" -exec ldd {} \;|awk '{print "cp ",$$3,"./ROOTFS/"}' |grep "/lib"|grep -v "ld-linux"|grep -v "libc.so"|grep -v "libresolv.so.2"|grep -v "libpthread.so"|grep -v "libdl.so"|grep -v "libm.so"|sort|uniq|sed -e "s/ //" >> tmp.sh
	sh tmp.sh
	rm tmp.sh

test:
	python test.py

clean:
	rm -rf upstream *.o *.so usr.manifest ROOTFS

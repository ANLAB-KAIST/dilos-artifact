.PHONY: all

OTP_VERSION=18.0
ERTS_VERSION=7.0

all: install erlang.so
module: install erlang.so

otp_src_$(OTP_VERSION).tar.gz:
	wget -c -O "$@.temp" "http://www.erlang.org/download/otp_src_$(OTP_VERSION).tar.gz"
	mv "$@.temp" "$@"

otp_src_$(OTP_VERSION): otp_src_$(OTP_VERSION).tar.gz
	tar xvf "$<"
	touch otp_src_$(OTP_VERSION)/lib/os_mon/c_src/disksup.c
	cd otp_src_$(OTP_VERSION); patch -b -p0 < ../os_mon.patch
	cd otp_src_$(OTP_VERSION); patch -b -p0 < ../epmd.patch

configure: otp_src_$(OTP_VERSION)
	cd otp_src_$(OTP_VERSION); export ERL_TOP=$(CURDIR)/otp_src_$(OTP_VERSION) CFLAGS=-fpie LDFLAGS="-pie -rdynamic"; ./configure -prefix=/usr --libdir=/usr/lib64 --disable-hipe --without-hipe --without-odbc --enable-builtin-zlib --with-termcap=ncurses --without-wx --without-erl_interface --without-javac --without-jinterface --with-ssl=$(CURDIR)/../../external/x64/misc.bin/usr

compile: configure
	mkdir -p ROOTFS
	cd otp_src_$(OTP_VERSION); export ERL_TOP=$(CURDIR)/otp_src_$(OTP_VERSION); $(MAKE)

install: compile
	cd otp_src_$(OTP_VERSION); export ERL_TOP=$(CURDIR)/otp_src_$(OTP_VERSION) DESTDIR=$(CURDIR)/ROOTFS; make install

erlang.so: erlang.c
	$(CC) -shared -rdynamic -fPIC -o erlang.so -std=gnu99 -DERTS_VERSION=$(ERTS_VERSION) erlang.c

clean:
	-rm -rf ROOTFS
	-rm -rf otp_src_$(OTP_VERSION)
	-rm -f otp_src_$(OTP_VERSION).tar.gz otp_src_$(OTP_VERSION).tar.gz.temp erlang.so

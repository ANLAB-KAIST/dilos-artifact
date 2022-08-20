.DEFAULT_GOAL := perl

PERLSRC_VERS = 5.20.2
PERLSRC_DIST = perl_$(PERLSRC_VERS).orig.tar.bz2
WGET_URL     = http://ftp.us.debian.org/debian/pool/main/p/perl/$(PERLSRC_DIST)

OSV_FLAGS       := -std=gnu99 -fpie -rdynamic -pie

CONFIGURE_FLAGS := -des -Duseshrplib -Ud_sigsetjmp -Dprefix=/osv -Dsiteprefix=/osv/opt -Dldflags="$(OSV_FLAGS)" -Dccflags=-fPIC
INSTALL_FLAGS   := DESTDIR=../ROOTFS

perl: perl/libperl.so

#install.perl target only installs perl, not manuals
install: perl extralibs
	cd perl && make $(INSTALL_FLAGS) install.perl

extralibs: perl/libperl.so
	mkdir -p ROOTFS/lib
	ldd perl/libperl.so | awk '/=> \// {print $$1, $$3}' | \
		while read target source; \
		do \
		case $$target in \
		libdl.*|libm.*|libc.*) ;; \
		*) cp -v $$source ROOTFS/lib/$$target;; \
		esac; \
		done

#ORIGINAL link step from debian:
#PERL_LDFLAGS    := -lnsl -ldl -lm -lcrypt -lutil -lc
#cc -o perl -fstack-protector -L/usr/local/lib -Wl,-E -Wl,-rpath,/usr/local/lib/perl5/5.20.2/x86_64-linux/CORE perlmain.o  libperl.so `cat ext.libs` $(PERL_LDFLAGS)

perl/libperl.so: perl/makefile
	cd perl && make

perl/makefile: $(PERLSRC_DIST)
	tar xf $(PERLSRC_DIST)
	mv perl-$(PERLSRC_VERS) perl
	cd perl && ./Configure $(CONFIGURE_FLAGS)

$(PERLSRC_DIST):
	wget -c $(WGET_URL)

#configure only (make clean configure):
configure:  perl/makefile

clean:
	rm -rf perl ROOTFS

deepclean: clean
	rm -f $(PERLSRC_DIST)

# for scripts/build
module: install

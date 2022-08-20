#
# Copyright (C) 2013 Cloudius Systems, Ltd.
#
# This work is open source software, licensed under the terms of the
# BSD license as described in the LICENSE file in the top-level directory.
#

.PHONY: module clean

jvm_dir=install/usr/lib/jvm

# FIXME: The following ugly RPM fetching code should be moved out of this
# Makefile and into a shell script, and duplication removed.

java_version=$(shell ./latest.sh java-1.8.0-openjdk-headless x86_64)
java_dir=http://mirrors.kernel.org/fedora/development/rawhide/Everything/x86_64/os/Packages/j/
java_fullname=java-1.8.0-openjdk-headless-$(java_version).x86_64.rpm

tzdata_version=$(shell ./latest.sh tzdata-java noarch)
tzdata_dir=http://mirrors.kernel.org/fedora/development/rawhide/Everything/x86_64/os/Packages/t/
tzdata_fullname=tzdata-java-$(tzdata_version).noarch.rpm

# More needed libraries by the JRE's libsunec.so - see issue #601
# "nspr" package provides libnspr4.so, libplds4.so and libplc4.so
nspr_version=$(shell ./latest.sh nspr x86_64)
nspr_dir=http://mirrors.kernel.org/fedora/development/rawhide/Everything/x86_64/os/Packages/n/
nspr_fullname=nspr-$(nspr_version).x86_64.rpm
# "nss" package providesa libnss3.so, libsmime3.so, libssl3.so
nss_version=$(shell ./latest.sh nss x86_64)
nss_dir=http://mirrors.kernel.org/fedora/development/rawhide/Everything/x86_64/os/Packages/n/
nss_fullname=nss-$(nss_version).x86_64.rpm
# "nss-util" package provides libnssutil3.so
nss-util_version=$(shell ./latest.sh nss-util x86_64)
nss-util_dir=http://mirrors.kernel.org/fedora/development/rawhide/Everything/x86_64/os/Packages/n/
nss-util_fullname=nss-util-$(nss-util_version).x86_64.rpm
# "nss-softokn-freebl" package provides libfreeblpriv3.so
nss-softokn-freebl_version=$(shell ./latest.sh nss-softokn-freebl x86_64)
nss-softokn-freebl_dir=http://mirrors.kernel.org/fedora/development/rawhide/Everything/x86_64/os/Packages/n/
nss-softokn-freebl_fullname=nss-softokn-freebl-$(nss-softokn-freebl_version).x86_64.rpm

# More needed libraries by the JRE's libjavalcms.so - see issue #601
# "lcms2" package provides liblcms2.so
lcms2_version=$(shell ./latest.sh lcms2 x86_64)
lcms2_dir=http://mirrors.kernel.org/fedora/development/rawhide/Everything/x86_64/os/Packages/l/
lcms2_fullname=lcms2-$(lcms2_version).x86_64.rpm


SRC = $(shell readlink -f ../..)

cpio_extract_option = $(shell cpio --help | grep -o '\-\-extract-over-symlinks')

module:
	mkdir -p upstream
	wget -c -O upstream/$(java_fullname) $(java_dir)$(java_fullname)
	wget -c -O upstream/$(tzdata_fullname) $(tzdata_dir)$(tzdata_fullname)
	wget -c -O upstream/$(nspr_fullname) $(nspr_dir)$(nspr_fullname)
	wget -c -O upstream/$(nss_fullname) $(nss_dir)$(nss_fullname)
	wget -c -O upstream/$(nss-util_fullname) $(nss-util_dir)$(nss-util_fullname)
	wget -c -O upstream/$(nss-softokn-freebl_fullname) $(nss-softokn-freebl_dir)$(nss-softokn-freebl_fullname)
	wget -c -O upstream/$(lcms2_fullname) $(lcms2_dir)$(lcms2_fullname)
	rm -rf install
	mkdir -p install/usr
	ln -s lib install/usr/lib64
	rpm2cpio upstream/$(java_fullname) | (cd install && cpio -id $(cpio_extract_option))
	# Move the versioned java directory to the generic "java" name, but
	# keep a link at the versioned name:
	mv $(jvm_dir)/java-1.8.0-openjdk-$(java_version).x86_64 $(jvm_dir)/java
	ln -sf java $(jvm_dir)/java-1.8.0-openjdk-$(java_version).x86_64
	rm -rf $(jvm_dir)/java/jre/lib/security/cacerts
	rm -rf $(jvm_dir)/java/jre/lib/audio/*
	rpm2cpio upstream/$(tzdata_fullname) | (cd install && cpio -id $(cpio_extract_option))
	rpm2cpio upstream/$(nspr_fullname) | (cd install && cpio -id $(cpio_extract_option))
	rpm2cpio upstream/$(nss_fullname) | (cd install && cpio -id $(cpio_extract_option))
	rpm2cpio upstream/$(nss-util_fullname) | (cd install && cpio -id $(cpio_extract_option))
	rpm2cpio upstream/$(nss-softokn-freebl_fullname) | (cd install && cpio -id $(cpio_extract_option))
	rpm2cpio upstream/$(lcms2_fullname) | (cd install && cpio -id $(cpio_extract_option))
	rm -rf install/usr/share/man
	ln -s /etc/pki/java/cacerts install/usr/lib/jvm/java/jre/lib/security/

clean:
	rm -rf upstream install

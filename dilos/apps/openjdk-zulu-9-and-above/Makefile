#
# Copyright (C) 2013 Cloudius Systems, Ltd.
#
# This work is open source software, licensed under the terms of the
# BSD license as described in the LICENSE file in the top-level directory.
#

.PHONY: module clean

JAVA_VERSION ?= 11
JAVA_PROFILE ?= normal

minimal_profile_modules = "java.base,jdk.unsupported"
# Add java.xml.bind to run spring boot
normal_profile_modules = "java.base,java.sql,java.logging,java.xml,jdk.unsupported,java.sql,java.naming,java.desktop,java.management,java.security.jgss,java.instrument,java.rmi"

profile_modules = $($(JAVA_PROFILE)_profile_modules)

jvm_dir=install/usr/lib/jvm

download_url=$(shell ./latest.sh $(JAVA_VERSION))
java_version=$(shell ./latest.sh $(JAVA_VERSION) version)
java_fullname=$(java_version)_bin.tar.gz

jre_name=jdk-$(java_version)
linked_jre_dir=usr/lib/jvm/$(jre_name)

SRC = $(shell readlink -f ../..)

module: upstream/$(java_fullname)
	rm -rf install && mkdir install
	cd install && ../upstream/$(java_version)/bin/jlink --module-path ../upstream/$(java_version)/jmods \
		--add-modules $(profile_modules) --output $(linked_jre_dir) --strip-debug --compress=2
	ln -sf $(jre_name) $(jvm_dir)/java
	rm -rf $(jvm_dir)/java/lib/security/cacerts
	ln -s /etc/pki/java/cacerts install/usr/lib/jvm/java/lib/security/
	mkdir -p mpm
	echo "name: osv.openjdk$(JAVA_VERSION)-zulu" > mpm/package.yaml
	printf "title: " >> mpm/package.yaml
	echo "Zulu Open JDK $(JAVA_VERSION)" >> mpm/package.yaml
	printf "version: " >> mpm/package.yaml
	install/usr/lib/jvm/java/bin/java --version 2>&1 | grep "openjdk" | grep -oP "1[^\"]+" >> mpm/package.yaml
	echo "require:" >> mpm/package.yaml
	echo "- osv.libz" >> mpm/package.yaml

upstream/$(java_fullname):
	mkdir -p upstream
	wget -c -O upstream/$(java_fullname) $(download_url)
	cd upstream && tar xfz $(java_fullname)

clean:
	rm -rf upstream install

#
# Copyright (C) 2013 Cloudius Systems, Ltd.
#
# This work is open source software, licensed under the terms of the
# BSD license as described in the LICENSE file in the top-level directory.
#

.PHONY: module clean

docker_image="adoptopenjdk/openjdk12:x86_64-ubuntu-jre-12.0.1_12"

SRC = $(shell readlink -f ../..)

module: upstream/undocker/.git
	rm -rf install && mkdir install
	docker pull $(docker_image)
	docker save $(docker_image) | upstream/undocker/undocker.py -o install
	$(SRC)/scripts/manifest_from_host.sh -r install opt/java/openjdk/ > usr.manifest
	echo '/java.so: ->/opt/java/openjdk/bin/java' >> usr.manifest

upstream/undocker/.git:
	mkdir -p upstream
	cd upstream && git clone --depth 1 https://github.com/larsks/undocker.git && chmod u+x undocker/undocker.py

clean:
	rm -rf upstream install usr.manifest

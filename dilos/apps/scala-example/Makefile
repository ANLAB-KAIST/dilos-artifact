#
# Copyright (C) 2014 Cloudius Systems, Ltd.
#
# This work is open source software, licensed under the terms of the
# BSD license as described in the LICENSE file in the top-level directory.
#

.PHONY: module

module: HelloWorld.jar

HelloWorld.jar: HelloWorld.scala
	rm -rf classes
	mkdir -p classes
	scalac -d classes HelloWorld.scala
	jar cf HelloWorld.jar -C classes .

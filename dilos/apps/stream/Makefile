#
# Copyright (C) 2014 Cloudius Systems, Ltd.
#
# This work is open source software, licensed under the terms of the
# BSD license as described in the LICENSE file in the top-level directory.
#

# "Official" STREAM homepage is at http://www.cs.virginia.edu/stream/ref.html
# This patch gets code from github.
url = https://github.com/jeffhammond/STREAM

SRCDIR=upstream/STREAM
module: ${SRCDIR}/stream ${SRCDIR}/stream.so

${SRCDIR}:
	mkdir -p upstream
	cd upstream; git clone https://github.com/jeffhammond/STREAM

${SRCDIR}/stream.c: ${SRCDIR}

${SRCDIR}/stream: ${SRCDIR}/stream.c
	gcc -O2 $^ -o $@

${SRCDIR}/stream.so: ${SRCDIR}/stream.c
	gcc -O2 $^ -o $@ -fPIC -shared

clean:
	rm -rf upstream

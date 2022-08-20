#!/bin/bash

# Copy required libraries from the build system.
# Likely, we don't want to include all libs (and their dependencies) from
# ldd ROOTFS/lib/python2.7/lib-dynload/*so | grep -v -e '^ROOTFS/' | sed 's/(0x.*)//' | sort | uniq | awk '{print $3}'
# So for now, only two (most obviously required) dependency libs will be included.

set -e

PYLIBS=""
PYLIBS+=" ROOTFS/lib/python2.7/lib-dynload/readline.so "
PYLIBS+=" ROOTFS/lib/python2.7/lib-dynload/_curses.so "

DEPLIBS=""
for pylib in $PYLIBS
do
	DEPLIBS+=`ldd $pylib | egrep -v -e linux-vdso.so -e ld-linux-x86-64.so -e 'lib(python[23]\.[0-9]|pthread|c|dl|m|util|).so.*'`
	DEPLIBS+="\n"
done
#echo -e "DEPLIBS1:\n$DEPLIBS"
DEPLIBS=`echo -e "$DEPLIBS" | sed '/^$/d' | awk '{print $3}' | sort | uniq`
#echo -e "DEPLIBS2:\n$DEPLIBS"

for deplib in $DEPLIBS
do
	/bin/cp $deplib ROOTFS/usr/lib
done

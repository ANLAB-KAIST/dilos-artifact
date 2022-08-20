#
# Copyright (C) 2018 Waldek Kozaczuk
#
# This work is open source software, licensed under the terms of the
# BSD license as described in the LICENSE file in the top-level directory.
#

SRCDIR=upstream
VERSION=4.2
DOWNLOAD_URL=https://www.ffmpeg.org/releases/
TARBALL=ffmpeg-${VERSION}.tar.xz
module: ROOTFS/ffmpeg.so

${SRCDIR}/ffmpeg-${VERSION}:
	mkdir -p upstream
	wget -c -O upstream/${TARBALL} ${DOWNLOAD_URL}/${TARBALL}
	cd upstream && tar xf ${TARBALL}

${SRCDIR}/ffmpeg-${VERSION}/ffmpeg: ${SRCDIR}/ffmpeg-${VERSION}
	# If needs x264 and x265 codex
	# sudo apt-get install libx264-dev libx265-dev
	cd ${SRCDIR}/ffmpeg-${VERSION} && ./configure --disable-all \
	--enable-ffmpeg \
	--enable-small \
	--enable-avcodec \
	--enable-avformat \
	--enable-avfilter \
	--enable-swresample \
	--enable-swscale \
	--enable-decoder=h264,h265 \
	--enable-encoder=rawvideo,libx264,libx265,png \
	--enable-parser=h264,h265 \
	--enable-protocol=file,http \
	--enable-demuxer=mov \
	--enable-muxer=rawvideo,mp4,mpegts,image2 \
	--enable-filter=scale \
	--enable-libx264 \
	--enable-libx265 \
	--enable-gpl
	patch ${SRCDIR}/ffmpeg-${VERSION}/fftools/ffmpeg.c fftools_ffmpeg.patch
	cd ${SRCDIR}/ffmpeg-${VERSION} && make
	cd ${SRCDIR}/ffmpeg-${VERSION} && rm ffmpeg ffmpeg_g fftools/*.o fftools/*.d
	patch ${SRCDIR}/ffmpeg-${VERSION}/fftools/Makefile fftools_makefile.patch
	cd ${SRCDIR}/ffmpeg-${VERSION} && make #Force rebuild ffmpeg executable as shared object

ROOTFS/ffmpeg.so: ${SRCDIR}/ffmpeg-${VERSION}/ffmpeg
	mkdir -p ROOTFS/usr/lib
	cp ${SRCDIR}/ffmpeg-${VERSION}/ffmpeg ROOTFS/ffmpeg.so
	find ${SRCDIR}/ffmpeg-${VERSION} -regex '.*\.so\.[0-9]+' | xargs -I {} cp {} ROOTFS/
	ldd ROOTFS/*.so* | grep -Po '(?<=> )/[^ ]+' | sort | uniq | grep -Pv 'lib(c|gcc_s|dl|m|util|rt|pthread|stdc\+\+|selinux|krb5|gssapi_krb5|z|bz2)\.so' | xargs -I {} install {} ROOTFS/usr/lib
	strip ROOTFS/ffmpeg.so && strip ROOTFS/usr/lib/*so*

clean:
	rm -rf upstream ROOTFS

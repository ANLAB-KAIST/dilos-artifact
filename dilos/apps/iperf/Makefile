#
# Copyright (C) 2014 Cloudius Systems, Ltd.
#
# This work is open source software, licensed under the terms of the
# BSD license as described in the LICENSE file in the top-level directory.
#

iperfv = iperf-2.0.5
tarball = $(iperfv).tar.gz
url = http://sourceforge.net/projects/iperf/files/$(tarball)/download

module: iperf

iperf: $(iperfv)/src/iperf
	cp $^ $@

$(iperfv)/src/iperf: $(tarball)
	tar xzf $^
	patch -p0 < so.patch
	cd $(iperfv) && CFLAGS='-fPIC -g' CXXFLAGS='-fPIC -g' ./configure
	+$(MAKE) -C $(iperfv)

$(tarball):
	wget -c -O $@ $(url)

clean:
	rm -rf $(tarball) $(iperfv) iperf

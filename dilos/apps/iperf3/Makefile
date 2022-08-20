#
# Copyright (C) 2014 Cloudius Systems, Ltd.
#
# This work is open source software, licensed under the terms of the
# BSD license as described in the LICENSE file in the top-level directory.
#

iperfv = iperf-3.6
tarball = $(iperfv).tar.gz
url = https://downloads.es.net/pub/iperf/$(tarball)

module: iperf3

iperf3: $(iperfv)/src/.libs/iperf3
	cp $^ $@
	cp $(iperfv)/src/.libs/libiperf.so.0.0.0 libiperf.so.0

$(iperfv)/src/.libs/iperf3: $(tarball)
	tar xzf $^
	cd $(iperfv) && CFLAGS='-fpie -g' CXXFLAGS='-fpie -g' LDFLAGS='-pie' ./configure --with-openssl=no
	+$(MAKE) -C $(iperfv)

$(tarball):
	wget -c -O $@ $(url)

clean:
	rm -rf $(tarball) $(iperfv) iperf3 libiperf.so.0

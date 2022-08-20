#
# Copyright (C) 2014 Cloudius Systems, Ltd.
#
# This work is open source software, licensed under the terms of the
# BSD license as described in the LICENSE file in the top-level directory.
#

detect_arch=$(shell echo $(1) | $(CC) -E -xc - | tail -n 1)

ifeq ($(call detect_arch, __x86_64__),1)
arch = x64
endif
ifeq ($(call detect_arch, __aarch64__),1)
arch = aarch64
endif

git_url = https://github.com/vladzcloudius/uperf
git_tag = v0.1

src_dir := $(notdir $(git_url))
TARGET = $(src_dir)/src/uperf

module: $(TARGET)

$(src_dir):
	git clone $(git_url) -b $(git_tag)

$(TARGET): $(src_dir)
	cd $< && ./configure --disable-sctp --enable-threads-only
	$(MAKE) -C $<
	$(MAKE) -C $< clean
	$(MAKE) -C $< CFLAGS=-fPIC LDFLAGS=-shared

clean:
	rm -rf $(src_dir)

.PHONY: clean module $(TARGET)

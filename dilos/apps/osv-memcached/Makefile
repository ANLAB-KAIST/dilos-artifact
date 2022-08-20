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

ifneq ($(OSV_BUILD_PATH),)
mode := $(notdir $(OSV_BUILD_PATH))
endif

git_url = https://github.com/vladzcloudius/osv-memcached
git_tag = v0.2

src_dir := $(notdir $(git_url))
TARGET = $(src_dir)/osv-memcached.so

module: $(TARGET)

$(src_dir):
	git clone $(git_url) -b $(git_tag)

$(TARGET): $(src_dir)
ifeq ($(OSV_BUILD_PATH),)
	@echo "OSV_BUILD_PATH not defined!"
	exit 1
endif
	$(MAKE) -C $(src_dir) OSV_PATH=../../.. MODE=$(mode) ARCH=$(arch)

clean:
	rm -rf $(src_dir)

.PHONY: clean module $(TARGET)

.PHONY: module clean

SRC = $(shell readlink -f ../..)
PYTHON_VERSION ?= 3

BASEDIR = $(shell pwd)
ROOTFS = $(BASEDIR)/ROOTFS

ifeq ($(PYTHON_VERSION),3)
PYTHON_EXEC := python3
else
PYTHON_EXEC := python
endif

python_path = $(shell which $(PYTHON_EXEC))
python_exe_path = $(shell realpath $(python_path)) 
python_base_name = $(shell basename $(python_exe_path))

PYTHON_PREFIX_DIR = $(shell $(PYTHON_EXEC) -c 'import sys; print(sys.prefix)')
PYTHON_MAJOR_VERSION = $(shell $(PYTHON_EXEC) -c 'import sys; print(sys.version_info.major)')
PYTHON_MINOR_VERSION = $(shell $(PYTHON_EXEC) -c 'import sys; print(sys.version_info.minor)')
PYTHON_MICRO_VERSION = $(shell $(PYTHON_EXEC) -c 'import sys; print(sys.version_info.micro)')
PYTHON_VERSION = $(PYTHON_MAJOR_VERSION).$(PYTHON_MINOR_VERSION)
FULL_PYTHON_VERSION = $(PYTHON_MAJOR_VERSION).$(PYTHON_MINOR_VERSION).$(PYTHON_MICRO_VERSION)

module:
	$(SRC)/scripts/manifest_from_host.sh $(python_exe_path) > usr.manifest
	rm -rf $(ROOTFS) && mkdir -p $(ROOTFS)/lib/python$(PYTHON_VERSION)
	for dir in $(PYTHON_PREFIX_DIR)/lib*/python$(PYTHON_VERSION)/ ; do \
		rsync -a $$dir $(ROOTFS)/lib/python$(PYTHON_VERSION)/ --safe-links --exclude test --exclude unittest \
		--exclude '*.pyc' --exclude '*.pyo' --exclude '*.egg-info' --exclude 'site-packages' --exclude 'dist-packages' ; \
	done
	$(SRC)/scripts/manifest_from_host.sh -r $(ROOTFS) >> usr.manifest
	echo '/python: ->/$(python_base_name)' >> usr.manifest
	mkdir -p mpm
	echo "name: osv.python$(PYTHON_MAJOR_VERSION)-from-host" > mpm/package.yaml
	echo "title: Python $(PYTHON_VERSION)" >> mpm/package.yaml
	echo "version: $(FULL_PYTHON_VERSION)" >> mpm/package.yaml

clean:
	rm -rf usr.manifest mpm

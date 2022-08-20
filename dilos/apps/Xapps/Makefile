.PHONY: module clean

SRC = $(shell readlink -f ../..)

xauthority_path := $(shell echo $$XAUTHORITY)

xclock_path := $(shell which xclock)
xeyes_path := $(shell which xeyes)
xlogo_path := $(shell which xlogo)

module:
	cp usr.manifest.skel usr.manifest
	echo '/.Xauthority: $(xauthority_path)' >> usr.manifest
	$(SRC)/scripts/manifest_from_host.sh $(xclock_path) >> usr.manifest
	echo '/xclock: $(xclock_path)' >> usr.manifest
	echo '/xeyes: $(xeyes_path)' >> usr.manifest
	echo '/xlogo: $(xlogo_path)' >> usr.manifest

clean:
	rm usr.manifest

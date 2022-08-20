.PHONY: module clean

SRC = $(shell readlink -f ../..)

ffmpeg_path = $(shell realpath $$(which ffmpeg)) 

module:
	$(SRC)/scripts/manifest_from_host.sh $(ffmpeg_path) > usr.manifest

clean:
	rm -rf usr.manifest

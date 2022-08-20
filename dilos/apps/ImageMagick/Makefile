.PHONY: module
module: usr.manifest convert.so

convert.so:
	cc -shared -o convert.so -fPIC `pkg-config --cflags ImageMagick` convert.c `pkg-config --libs MagickWand`

genlib = ldd convert.so | grep "lib$1.so" | sed 's/ *[^ ] *\(.*\) => \(.*\) .*/\/\1: \2/'

usr.manifest: convert.so
	: > usr.manifest
	echo '/usr/bin/convert:$${MODULE_DIR}/convert.so' >> usr.manifest
	$(call genlib,Magick.*) >> usr.manifest
	$(call genlib,gomp.*) >> usr.manifest
	$(call genlib,ltdl.*) >> usr.manifest

.PHONY: clean
clean:
	rm -f convert.so usr.manifest

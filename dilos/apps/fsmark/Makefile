.PHONY: module
module: fsmark.so
	echo '/fsmark.so: $${MODULE_DIR}/fsmark.so' > usr.manifest
	: > bootfs.manifest

fsmark.so: upstream
	make -C upstream/fs_mark CFLAGS="-fPIC -shared" CPPFLAGS="-fPIC -shared"
	cp -vf upstream/fs_mark/fs_mark fsmark.so

upstream:
	./GET

.PHONY: clean
clean:
	rm -rf upstream *.so usr.manifest bootfs.manifest *~

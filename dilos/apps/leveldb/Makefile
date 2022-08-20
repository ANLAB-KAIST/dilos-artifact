.PHONY: module
module: src
	echo '/db_bench.so: $${MODULE_DIR}/db_bench.so' > usr.manifest
	echo '/usr/lib/libleveldb.so.1.15: $${MODULE_DIR}/libleveldb.so.1.15' >> usr.manifest
	echo '/usr/lib/libsnappy.so.1: /lib64/libsnappy.so.1' >> usr.manifest
	: > bootfs.manifest

.PHONY: src
src:
	./GET

clean:
	rm -rf leveldb-* *.tar.gz *.so leveldb *.manifest libleveldb.* ROOTFS

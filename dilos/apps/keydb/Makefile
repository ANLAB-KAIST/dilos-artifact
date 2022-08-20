SRC = $(shell readlink -f ../..)

upstream/KeyDB/.git:
	mkdir -p upstream
	cd upstream && git clone -b v5.3.1 --depth 1 https://github.com/JohnSully/KeyDB.git
	cd $(SRC)/apps/keydb && patch upstream/KeyDB/src/server.cpp proctitle.patch

upstream/KeyDB/src/keydb-server: upstream/KeyDB/.git
	cd upstream/KeyDB && make MALLOC=libc

module: upstream/KeyDB/src/keydb-server
	$(SRC)/scripts/manifest_from_host.sh upstream/KeyDB/src/keydb-server > usr.manifest
	strip upstream/KeyDB/src/keydb-server

clean:
	rm -rf upstream usr.manifest

.PHONY: module clean

SRC = $(shell readlink -f ../..)

node_exe_path = $(shell realpath $$(which node)) 

module:
	echo "/libnode.so: ->/node" > usr.manifest
	$(SRC)/scripts/manifest_from_host.sh $(node_exe_path) >> usr.manifest
	mkdir -p mpm
	echo "title: Node JS" > mpm/package.yaml
	printf "version: " >> mpm/package.yaml
	node --version >> mpm/package.yaml

clean:
	rm -rf usr.manifest mpm

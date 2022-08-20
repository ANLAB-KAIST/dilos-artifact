src = $(shell readlink -f ../..)
app-dir = $(src)/apps/webassembly
wasmer-version = 0.11.0

all: module

module: $(app-dir)/install/wasmer examples

$(app-dir)/install/wasmer:
	mkdir -p $(app-dir)/upstream
	wget -c -O $(app-dir)/upstream/wasmer-linux-amd64.tar.gz https://github.com/wasmerio/wasmer/releases/download/$(wasmer-version)/wasmer-linux-amd64.tar.gz
	mkdir -p $(app-dir)/install
	cd $(app-dir)/upstream && tar -xf wasmer-linux-amd64.tar.gz
	mv $(app-dir)/upstream/bin/wasmer $(app-dir)/install/wasmer
	strip $(app-dir)/install/wasmer

examples: $(app-dir)/upstream/sqlite.wasm $(app-dir)/upstream/lua.wasm $(app-dir)/upstream/nginx.wasm

$(app-dir)/upstream/%.wasm:
	$(makedir)
	wget -c -O $(app-dir)/upstream/$*.wasm https://github.com/wasmerio/wasmer/raw/master/examples/$*.wasm

$(app-dir)/upstream/nginx.wasm:
	wget -c -O $(app-dir)/upstream/nginx.wasm https://github.com/wasmerio/wasmer/raw/master/examples/nginx/nginx.wasm

clean:
	cd $(app-dir) && rm -rf upstream && rm -rf install

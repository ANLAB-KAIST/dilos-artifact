src = $(shell readlink -f ../..)
app-dir = $(src)/apps/deno
deno-version = 0.24.0

all: module

module: $(app-dir)/install/deno_linux_x64
	echo "/deno_linux_x64 : $(app-dir)/install/deno_linux_x64" > usr.manifest
	echo "/examples/**: $(app-dir)/examples/**" >> usr.manifest

$(app-dir)/install/deno_linux_x64:
	mkdir -p $(app-dir)/upstream
	wget -c -O $(app-dir)/upstream/deno_linux_x64.gz https://github.com/denoland/deno/releases/download/v$(deno-version)/deno_linux_x64.gz
	mkdir -p $(app-dir)/install
	gzip -cd $(app-dir)/upstream/deno_linux_x64.gz > $(app-dir)/install/deno_linux_x64
	strip $(app-dir)/install/deno_linux_x64

clean:
	cd $(app-dir) && rm -rf upstream && rm -f usr.manifest && rm -rf install

src = $(shell readlink -f ../..)
app-dir = $(src)/apps/ratpack-example

all: module
module: ratpack-example

module: $(app-dir)/upstream/tutorials/.git
	cd $(app-dir)/upstream/tutorials/ratpack && mvn -Dgib.enabled=false package
	mkdir -p $(app-dir)/ROOTFS && cd $(app-dir)/ROOTFS && jar xf ../upstream/tutorials/ratpack/target/ratpack.jar
	echo "/ratpack-app/**: $(app-dir)/ROOTFS/**" > $(app-dir)/usr.manifest

.PHONY: ratpack-example

$(app-dir)/upstream/tutorials/.git:
	mkdir -p $(app-dir)/upstream
	cd $(app-dir)/upstream && git clone --depth 1 https://github.com/eugenp/tutorials.git
	patch $(app-dir)/upstream/tutorials/ratpack/pom.xml pom.patch

clean:
	cd $(app-dir) && rm -rf upstream ROOTFS usr.manifest

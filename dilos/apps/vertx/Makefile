src = $(shell readlink -f ../..)
app-dir = $(src)/apps/vertx
vertx-version = $(shell grep -P "<version>(\d\.)+\d</version>" vertx-examples/pom.xml | grep -Po "(\d\.)+\d")

all: module
module: vertx

module: vertx-examples/.git
	mkdir -p $(app-dir)/vertx-examples/maven-simplest/src/main/resources
	cp $(app-dir)/vertx-default-jul-logging.properties $(app-dir)/vertx-examples/maven-simplest/src/main/resources/
	cd $(app-dir)/vertx-examples/maven-simplest && mvn package
	echo "/HelloWorld.jar : $(app-dir)/vertx-examples/maven-simplest/target/maven-simplest-$(vertx-version)-fat.jar" > $(app-dir)/usr.manifest

.PHONY: vertx

vertx-examples/.git:
	cd $(app-dir) && git clone --depth 1 https://github.com/vert-x3/vertx-examples.git

clean:
	cd $(app-dir) && rm -rf vertx-examples && rm -f usr.manifest

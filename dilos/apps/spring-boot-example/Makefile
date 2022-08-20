src = $(shell readlink -f ../..)
app-dir = $(src)/apps/spring-boot-example
example = spring-boot-2-rest-service-basic
example-dir = upstream/spring-boot-examples/$(example)

all: module
module: spring-boot-example

module: upstream/spring-boot-examples/.git
	cp pom.xml $(app-dir)/$(example-dir)/pom.xml
	cd $(app-dir)/$(example-dir) && mvn package
	echo "/$(example).jar : $(app-dir)/$(example-dir)/target/$(example)-0.0.1-SNAPSHOT.jar" > $(app-dir)/usr.manifest

.PHONY: spring-boot-example

upstream/spring-boot-examples/.git:
	mkdir -p upstream
	cd $(app-dir)/upstream && git clone --depth 1 https://github.com/in28minutes/spring-boot-examples.git

clean:
	cd $(app-dir) && rm -rf upstream usr.manifest

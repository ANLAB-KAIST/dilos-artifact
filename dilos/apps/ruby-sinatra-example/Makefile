.PHONY: module
module: ROOTFS

SINATRA_VER=1.4.5

ROOTFS/sinatra/gems:
	mkdir -p ROOTFS/sinatra/gems
	GEM_HOME=ROOTFS/sinatra/gems gem install sinatra -v ${SINATRA_VER}

ROOTFS/sinatra/example.rb:
	mkdir -p ROOTFS/sinatra
	cp example.rb ROOTFS/sinatra/

ROOTFS: ROOTFS/sinatra/gems ROOTFS/sinatra/example.rb

clean:
	rm -rf ROOTFS

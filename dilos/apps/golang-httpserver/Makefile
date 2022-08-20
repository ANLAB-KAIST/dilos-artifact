.PHONY: module
module: httpserver.so
	echo '/httpserver.so: $${MODULE_DIR}/httpserver.so' > usr.manifest

httpserver.so: httpserver.go
	go build -a -buildmode=c-shared -o httpserver.so httpserver.go

clean:
	rm -f httpserver*.so httpserver.h usr.manifest

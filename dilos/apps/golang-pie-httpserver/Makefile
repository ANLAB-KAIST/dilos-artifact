.PHONY: module
module: httpserver.so
	echo '/httpserver.so: $${MODULE_DIR}/httpserver.so' > usr.manifest

httpserver.so: httpserver.go
	go build -buildmode=pie -o httpserver.so httpserver.go

clean:
	rm -f httpserver*.so usr.manifest

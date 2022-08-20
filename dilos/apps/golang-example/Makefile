.PHONY: module
module: hello.so
	echo '/hello.so: $${MODULE_DIR}/hello.so' > usr.manifest

hello.so: hello.go
	go build -buildmode=c-shared -o hello.so hello.go

clean:
	rm -f hello*.so hello.h usr.manifest

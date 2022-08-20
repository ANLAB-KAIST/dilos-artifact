.PHONY: module
module: hello.so
	echo '/hello.so: $${MODULE_DIR}/hello.so' > usr.manifest
	ldd hello.so | grep libgo.so | \
		sed 's/ *[^ ] *\(.*\) => \(.*\) .*/\/\1: \2/' \
		>> usr.manifest

CFLAGS = -g -fPIC
LDFLAGS = -lgo -lgobegin

%.o: %.go
	gccgo -c $(CFLAGS) -o $@ $<

hello.so: hello.o go-main.o
	$(CC) -shared -o $@ $^ $(LDFLAGS)

clean:
	rm -f hello.so *.o hello usr.manifest

# Test pie (works on Linux, but not yet on OSv)
hello: hello.o
	$(CC) -pie -o $@ $^ $(LDFLAGS)

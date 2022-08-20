.PHONY: module
module: hello

CFLAGS = -std=gnu99 -fpie -rdynamic

hello: hello.c
	$(CC) -pie -o $@ $(CFLAGS) $(LDFLAGS) hello.c

clean:
	rm -f hello

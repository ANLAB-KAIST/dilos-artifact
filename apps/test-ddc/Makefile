.PHONY: module


CFILES = $(wildcard *.c)

TESTS = $(patsubst %.c,test-%, ${CFILES})

module: $(TESTS) usr.manifest

CFLAGS = -fpie  -std=gnu99 -I../../dilos/include -L../../dilos/build/last -losv

test-%: %.c Makefile
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $<

usr.manifest: $(TESTS) Makefile
	@rm -f usr.manifest
	@for test in $(TESTS) ; do \
        echo "/$$test: \$${MODULE_DIR}/$$test" >> usr.manifest; \
    done

clean:
	rm -f test-* usr.manifest

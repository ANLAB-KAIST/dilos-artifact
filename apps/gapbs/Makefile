
ALGOS = bc bfs cc pr tc
BINS = $(addprefix gapbs/,$(ALGOS))
genlib = ldd gapbs/bfs | grep "lib$1.so" | sed 's/ *[^ ] *\(.*\) => \(.*\) .*/\/\1: \2/'

.PHONY: module
module: $(BINS) usr.manifest


.PHONY: build-all
build-all:
	cd gapbs; make -j

.PHONY: $(BINS)
$(BINS): build-all

.PHONY: clean
clean:
	cd gapbs; make clean


usr.manifest: $(BINS) Makefile
	@rm -f usr.manifest
	@for algo in $(ALGOS) ; do \
        echo "/$$algo: \$${MODULE_DIR}/gapbs/$$algo" >> usr.manifest; \
    done
	$(call genlib,gomp.*) >> usr.manifest
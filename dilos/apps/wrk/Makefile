version = 3.1.0
tarball = $(version).tar.gz

.PHONY: module
module: wrk
	$(MAKE) -C wrk

$(tarball):
	wget https://github.com/wg/wrk/archive/$(version).tar.gz

wrk: $(tarball)
	tar xvzf $(tarball)
	mv wrk-$(version) wrk
	cd wrk && patch -p1 < ../0001-make-build-as-DSO.patch
	cd wrk && patch -p1 < ../wrk-fix-uninit-connection.patch
	cd wrk && patch -p1 < ../0003-wrk-allow-larger-file-descriptor-values.patch

clean:
	rm -rf wrk $(tarball)

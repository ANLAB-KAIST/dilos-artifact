.PHONY: module

INCLUDES=-I$(OSV_BASE)/include
CFLAGS=--std=c++11 -shared -fPIC -lrpm $(INCLUDES)
CXX=g++
OVERSION=$(shell $(OSV_BASE)/scripts/osv-version.sh | tr - .)

module: upstream rpm.so noarch/osv-$(OVERSION)-1.noarch.rpm

rpm.so: rpm.cc
	$(CXX) $(CFLAGS) $< -o $@

upstream:
	./GET

.PHONY: osvrpm
noarch/osv-$(OVERSION)-1.noarch.rpm: osv.spec
	rpmbuild -bb osv.spec -D"osversion $(OVERSION)" -D"_rpmdir ."; \
	echo "/rpms/osv-$(OVERSION)-1.noarch.rpm: $$PWD/noarch/osv-$(OVERSION)-1.noarch.rpm" >> usr.manifest

.PHONY: clean
clean:
	rm -rf upstream usr.manifest noarch rpm.so *~

.DEFAULT_GOAL := install

_CADO_VERS := 1_107
CADO_VERS  != echo 1_107 | sed -e 's/_/./'
CADOSRC_DIST  := cado_linux_$(_CADO_VERS).tgz
WGET_URL      := http://iweb.dl.sourceforge.net/project/cado/v$(CADO_VERS)/$(CADOSRC_DIST)

install: ROOTFS/cado

$(CADOSRC_DIST):
	@echo CADO_VERS=$(CADO_VERS) CADOSRC_DIST=$(CADOSRC_DIST)
	wget -c $(WGET_URL)

ROOTFS/cado: $(CADOSRC_DIST)
	mkdir -p ROOTFS
	tar xf $(CADOSRC_DIST)
	mv cado_linux_$(_CADO_VERS) ROOTFS/cado
	#punt on compiled crc for now since we can't fork:
	rm -rf ROOTFS/cado/bin/linux
	#temporary fix for sp.pl:
	chmod 0644 ROOTFS/cado/lib/cmn/sp.pl
	patch -i patch.sp.pl ROOTFS/cado/lib/cmn/sp.pl
	#remove shell exec header for cado:
	chmod 0644 ROOTFS/cado/bin/cmn/cado
	sed -ne '2,$$p' ROOTFS/cado/bin/cmn/cado > ROOTFS/cado/bin/cmn/cado.new
	mv ROOTFS/cado/bin/cmn/cado.new ROOTFS/cado/bin/cmn/cado 

clean:
	rm -rf ROOTFS

deepclean: clean
	rm -f $(CADOSRC_DIST)

# for scripts/build
module: install

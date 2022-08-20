.PHONY: all

ELIXIR_VERSION=1.0.5

all: install
module: install

elixir-$(ELIXIR_VERSION).tar.gz:
	wget -c -O "$@.temp" "https://github.com/elixir-lang/elixir/archive/v$(ELIXIR_VERSION).tar.gz"
	mv "$@.temp" "$@"

elixir-$(ELIXIR_VERSION): elixir-$(ELIXIR_VERSION).tar.gz
	tar xvf "$<"

compile: elixir-$(ELIXIR_VERSION)
	make -C elixir-$(ELIXIR_VERSION) clean
	make -C elixir-$(ELIXIR_VERSION)

install: compile
	export DESTDIR=$(CURDIR)/ROOTFS; make -C elixir-$(ELIXIR_VERSION) install

clean:
	-rm -rf ROOTFS
	-rm -rf elixir-$(ELIXIR_VERSION)
	-rm -rf elixir-$(ELIXIR_VERSION).tar.gz elixir-$(ELIXIR_VERSION).tar.gz.temp

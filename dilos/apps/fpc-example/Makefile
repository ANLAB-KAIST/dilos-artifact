.PHONY: module
module: fpc-hello


fpc-hello: fpc-hello.pas
	fpc -fPIC -XD -Xc -s -ofpc-hello fpc-hello.pas
	# if we drop -init and -fini from link line, it works. Like this it doesn't, currently.
	sh ppas.sh

clean:
	rm -f fpc-hello fpc-hello.o link.res ppas.sh

.PHONY: module
module: seq
 
arch = x64
ifndef mode
	mode = release
endif

src = $(shell readlink -f ../..)



CFLAGS = $(INCLUDES) -O3 -pthread

seq: seq.cc
	$(CXX) -o $@ $(CFLAGS) $(LDFLAGS) seq.cc


clean:
	rm -f seq

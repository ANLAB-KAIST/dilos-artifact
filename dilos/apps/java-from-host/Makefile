.PHONY: module clean

SRC = $(shell readlink -f ../..)

java_exe_path = $(shell realpath $$(which java)) 
java_bin_path = $(shell dirname $(java_exe_path))
java_jre_path = $(shell dirname $(java_bin_path))

module:
	echo "/java.so: ->$(java_exe_path)" > usr.manifest
	echo "/usr/bin/java: ->$(java_exe_path)" > usr.manifest
	$(SRC)/scripts/manifest_from_host.sh -rR $(java_jre_path) | grep -vi libX >> usr.manifest

clean:
	rm -rf usr.manifest

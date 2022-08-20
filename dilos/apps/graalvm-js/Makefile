include ../graalvm-example/common.gmk

module: $(app-dir)/upstream/graalvm-ce-java11-$(GRAAL_VERSION)/bin/js
	strip $(app-dir)/upstream/graalvm-ce-java11-$(GRAAL_VERSION)/bin/js
	echo "/js: $(app-dir)/upstream/graalvm-ce-java11-$(GRAAL_VERSION)/bin/js" > usr.manifest

clean:
	rm -f usr.manifest

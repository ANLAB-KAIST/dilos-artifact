.PHONY: module
module: memcached
	echo '/memcached: $${MODULE_DIR}/memcached' > usr.manifest
	ldd memcached | grep libevent | \
		sed 's/ *[^ ] *\(.*\) => \(.*\) .*/\/\1: \2/' \
		>> usr.manifest
	: > bootfs.manifest

src:
	./GET

sources := assoc.c cache.c daemon.c hash.c items.c memcached.c slabs.c \
	stats.c thread.c util.c jenkins_hash.c murmur3_hash.c

sources := $(addprefix src/, $(sources))

$(sources): src

CFLAGS = -std=gnu99 -fpie -rdynamic -DHAVE_CONFIG_H
LDFLAGS = -pie
# To use a shared object instead of PIE (e.g., we needed to do this because
# of issue #689), use the following instead:
#CFLAGS = -std=gnu99 -fpic -rdynamic -DHAVE_CONFIG_H
#LDFLAGS = -shared

memcached: $(sources)
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $(sources) -levent -lpthread

clean:
	rm -rf upstream src
	rm -f bootfs.manifest usr.manifest module.pyc memcached

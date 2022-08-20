.PHONY: module
module: redis/src/redis-server

# EXTRA_ARGS=CFLAGS="-DREDIS_STAT -L$(shell pwd)/../../dilos/build/last -losv"
EXTRA_ARGS=CFLAGS="-fPIC -L$(shell pwd)/../../dilos/build/last -losv" LDFLAGS="-shared"


.PHONY: redis/src/redis-server
redis/src/redis-server:
	cd redis; make MALLOC=libc $(EXTRA_ARGS) redis-server

.PHONY: clean
clean:
	cd redis; make clean
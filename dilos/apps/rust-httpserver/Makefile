.PHONY: module
module: target/release/libhttpserver.so
	echo '/libhttpserver.so: $${MODULE_DIR}/target/release/libhttpserver.so' > usr.manifest

target/release/libhttpserver.so: src/lib.rs
	(command -v cargo && cargo build --release) || echo "Please install Rust to build this app"

clean:
	(command -v cargo && cargo clean) || echo > /dev/null
	rm -f usr.manifest

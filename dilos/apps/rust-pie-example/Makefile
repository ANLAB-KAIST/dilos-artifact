.PHONY: module
module: target/release/hello
	echo '/hello: $${MODULE_DIR}/target/release/hello' > usr.manifest

target/release/hello: src/main.rs
	cargo --version && cargo build --release || echo "Please install Rust to build this app"

clean:
	-cargo clean
	rm -f usr.manifest

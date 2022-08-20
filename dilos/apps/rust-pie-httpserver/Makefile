.PHONY: module
module: target/release/httpserver
	echo '/httpserver: $${MODULE_DIR}/target/release/httpserver' > usr.manifest

target/release/httpserver: src/main.rs
	(command -v cargo && cargo build --release) || echo "Please install Rust to build this app"

clean:
	(command -v cargo && cargo clean) || echo > /dev/null
	rm -f usr.manifest

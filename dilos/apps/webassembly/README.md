Wasmer is a Universal WebAssembly Runtime that allows running `*.was`
binaries as well as `*.wat` text files on Linux. Wasmer is implemented in Rust. 

This apps contains examples of 3 WASM apps - lua, sqlite and nginx.

For more info look at https://wasmer.io/.

Please note that like all apps written in Rust, wasmer uses pretty large TLS
so in order to run it on OSv you nead to link loader.elf with big enough
local TLS reservation using proper value of `app_local_exec_tls_size` parameter
when passing it to the `./scripts/build'. Currently the values is `2408`.

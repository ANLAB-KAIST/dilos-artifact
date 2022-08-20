Deno (No-de reversed, I guess;-) is a new Typescript/Javascript runtime
implemented in Rust and using V8 as Javascript language.

For more info look at https://deno.land/.

Please note that like all apps written in Rust, deno uses pretty large TLS
so in order to run it on OSv you nead to link loader.elf with big enough
local TLS reservation using proper value of `app_local_exec_tls_size` parameter
when passing it to the `./scripts/build'. Currently the values is `5608`.

Example to run httpserver:
```
wrk -d 10 -c 100 http://localhost:8000/
```

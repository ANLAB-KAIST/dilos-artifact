from osv.modules import api

default = api.run('/deno_linux_x64')
httpserver = api.run('--env=DENO_DIR=/tmp/deno /deno_linux_x64 --allow-net /examples/httpserver.ts')

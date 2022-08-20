from osv.modules import api

default = api.run('/wasmer --help')

lua = api.run('/wasmer run /lua.wasm')
sqlite = api.run('/wasmer run /sqlite.wasm')
nginx = api.run('/wasmer run /nginx/nginx.wasm -- -p /nginx -c /nginx/nginx.conf')

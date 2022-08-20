from osv.modules import api

api.require('libz')
default = api.run('/nginx.so -c /nginx/conf/nginx.conf')

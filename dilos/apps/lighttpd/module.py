from osv.modules import api

default = api.run('/lighttpd.so -D -f /lighttpd/lighttpd.conf')

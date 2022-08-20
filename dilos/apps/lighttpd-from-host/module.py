from osv.modules import api

default = api.run('/lighttpd -D -f /lighttpd.conf')

from osv.modules import api
api.require("lua")
default = api.run("/usr/lib/liblua.so /usr/lib/hello.lua")

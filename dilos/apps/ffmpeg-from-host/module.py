from osv.modules import api

api.require("libz")
default = api.run("/ffmpeg -formats")

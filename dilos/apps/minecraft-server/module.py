from osv.modules import api

api.require('java')

default = api.run('java.so -Xms1024M -Xmx1024M -jar minecraft_server.1.7.10.jar nogui')

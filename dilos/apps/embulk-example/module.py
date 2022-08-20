from osv.modules import api

api.require('java')

default = api.run(cmdline="--env=HOME=/ java.so -jar /embulk.jar run config.yml")


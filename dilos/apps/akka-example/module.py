from osv.modules import api

api.require('java8')

default = api.run("/java.so -jar akka-example.jar")

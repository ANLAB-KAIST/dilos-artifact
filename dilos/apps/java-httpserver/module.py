from osv.modules import api

api.require('java')

default = api.run_java(classpath=['/java-httpserver'], args=['HttpServerApp'])

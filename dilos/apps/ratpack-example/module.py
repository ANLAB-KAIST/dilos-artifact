from osv.modules import api

api.require('java8')

default = api.run('/java.so -cp /ratpack-app org.springframework.boot.loader.JarLauncher')

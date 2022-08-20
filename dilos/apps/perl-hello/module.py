from osv.modules import api

api.require('perl')

default = api.run(cmdline="/osv/bin/perl hello.pl")

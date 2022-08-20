from osv.modules import api

default = api.run(cmdline="/memcached -t $OSV_CPUS -u root")

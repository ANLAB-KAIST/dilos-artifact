import os
from osv.modules.api import *
from osv.modules.filemap import FileMap
from osv.modules import api

_app = '${OSV_BASE}/apps/httpserver-html5-cli'

usr_files = FileMap()
usr_files.add(os.path.join(_app, 'osv-html5-terminal/dist')).to('/usr/mgmt/cli')
usr_files.add(os.path.join(_app, 'httpserver.conf')).to('/etc/httpserver.conf')

api.require('httpserver-api')

# httpserver will run regardless of an explicit command line
# passed with "run.py -e".
_exe = '/libhttpserver-api.so --config-file=/etc/httpserver.conf'
daemon = api.run_on_init(_exe + ' &!')

fg = api.run(_exe)

fg_ssl = api.run(_exe + ' --ssl')
fg_cors = api.run(_exe + ' --access-allow=true')

default = daemon

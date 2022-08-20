import os
from osv.modules.api import *
from osv.modules.filemap import FileMap
from osv.modules import api

_module = '${OSV_BASE}/modules/httpserver-api'

_exe = '/libhttpserver-api.so'

usr_files = FileMap()
usr_files.add(os.path.join(_module, 'libhttpserver-api.so')).to(_exe)
usr_files.add(os.path.join(_module, 'api-doc')).to('/usr/mgmt/api')

api.require('openssl')
api.require('libtools')

# only require next 3 modules if java (jre) is included in the list of modules
api.require_if_other_module_present('josvsym','java')
api.require_if_other_module_present('httpserver-jolokia-plugin','java')
api.require_if_other_module_present('httpserver-jvm-plugin','java')

# httpserver will run regardless of an explicit command line
# passed with "run.py -e".
daemon = api.run_on_init(_exe + ' &!')
daemon_ssl = api.run_on_init(_exe + ' --ssl &!')

fg = api.run(_exe)

fg_ssl = api.run(_exe + ' --ssl')

default = daemon

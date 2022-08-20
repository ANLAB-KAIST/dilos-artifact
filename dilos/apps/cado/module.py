from osv.modules import api

api.require('perl')

default = api.run(
    '--cwd=/cado '
    '--env=PATH=/osv/bin:/cado/bin/cmn '
    '--env=PERL_LIBPATH=/cado/lib/cmn '
    '/osv/bin/perl '
    '/cado/bin/cmn/cado -V'
)

from osv.modules import api

api.require('fonts')

common_prefix = '--env=XAUTHORITY=/.Xauthority --env=DISPLAY=192.168.122.1:0.0'

default = api.run(cmdline='%s /xclock -digital -fg red -bg black -update 1 -face "Arial Black-25:bold"' % common_prefix)
logo = api.run(cmdline='%s /xlogo' % common_prefix)
calc = api.run(cmdline='%s /xcalc' % common_prefix)
eyes = api.run(cmdline='%s /xeyes' % common_prefix)

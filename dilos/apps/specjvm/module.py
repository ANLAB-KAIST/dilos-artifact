from osv.modules import api

api.require('java')

#Run with --help to see what all options mean
#Warmup 30s, 1 iteration of 30s with 2 benchmark threads
common_cmd = '/usr/bin/java -Xms1400m -Xmx1400m -jar specjvm.jar -wt 30s -it 30s -bt 2 -i 1 -ikv -ict -ctf false -chf false -crf false'

all = api.run(cmdline='%s compress crypto serial sunflow scimark!' % common_cmd)

derby = api.run(cmdline='%s derby!' % common_cmd) #Need ZFS image with 1GB space at least
xml = api.run(cmdline='%s xml!' % common_cmd) #Need ZFS image

default = api.run(cmdline='%s sunflow!' % common_cmd)

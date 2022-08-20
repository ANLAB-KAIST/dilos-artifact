import sys
import subprocess
import signal
import time
import re
import urllib
import urllib2

def do_http_test():
    time.sleep(3)
    resp = urllib2.urlopen('http://192.168.122.89:3000/setup')
    body = resp.read()
    print("response of /setup:%s" % body)
    if re.search(r'Welcome to your blog setup', body) == None:
        return False

    return True

qemu = subprocess.Popen("./scripts/run.py -n", shell=True, cwd="../..", stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
while True:
    line = qemu.stdout.readline().strip()
    print(line)

    if re.search(r'Listening on 0.0.0.0:3000', line) is not None:
        result =  do_http_test()
        subprocess.check_call(['killall', '-s', 'SIGINT', 'qemu-system-x86_64'])
        qemu.wait()
        if result == True:
            break
        else:
            sys.exit(1)

    if re.search(r'Aborted', line) is not None:
        subprocess.check_call(['killall', '-s', 'SIGKILL', 'qemu-system-x86_64'])
        qemu.wait()
        sys.exit(1)

    if qemu.poll() is not None:
        print("qemu died")
        sys.exit(1)

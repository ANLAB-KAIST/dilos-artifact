import sys
import subprocess
import signal
import re
import urllib
import urllib2

def do_http_test():
    resp = urllib2.urlopen('http://192.168.122.89:3000/')
    body = resp.read()
    print("response of /:%s" % body)
    if re.search(r'Hello World', body) == None:
        return False
    try:
        resp = urllib2.urlopen('http://192.168.122.89:3000/invalid_url')
    except urllib2.HTTPError, e:
        print("e.code:%d" % e.code)
        if e.code == 404:
            return True
    except:
        return False
    return False


qemu = subprocess.Popen("./scripts/run.py -n", shell=True, cwd="../..", stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
while True:
    line = qemu.stdout.readline().strip()
    print(line)

    if re.search(r'Express started on port 3000', line) is not None:
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

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
    if re.search(r'You&rsquo;re riding Ruby on Rails!', body) == None:
        return False
    
    resp = urllib2.urlopen('http://192.168.122.89:3000/items')
    body = resp.read()
    print("response of /items:%s" % body)
    if re.search(r'Listing items', body) == None:
        return False
    
    resp = urllib2.urlopen('http://192.168.122.89:3000/items/new')
    body = resp.read()
    print("response of /items/new:%s" % body)
    if re.search(r'Description', body) == None:
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

    if re.search(r'.+WEBrick::HTTPServer#start', line) is not None:
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

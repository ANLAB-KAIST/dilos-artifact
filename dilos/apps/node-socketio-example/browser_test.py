import sys
import subprocess
import signal
import re
from selenium import webdriver

def do_browser_test():
    try:
        driver = webdriver.Firefox()

        driver.get('http://192.168.122.89:3000/')
        for i in range(1, 100):
            driver.implicitly_wait(3) # seconds
            input = driver.find_element_by_id('m')
            input.send_keys(u'msg%d' % i)
            input.submit()

        i = 1
        driver.implicitly_wait(3) # seconds
        for li in driver.find_elements_by_tag_name('li'):
            assert li.text == str('msg%d' % i)
            i += 1
        return True
    except Exception, e:
        print(e)
        return False

qemu = subprocess.Popen("./scripts/run.py -n", shell=True, cwd="../..", stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
while True:
    line = qemu.stdout.readline().strip()
    print(line)

    if re.search(r'listening on ', line) is not None:
        result =  do_browser_test()
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

import sys
import subprocess
import signal
import re
from selenium import webdriver

def do_browser_test():
    try:
        driver = webdriver.Firefox()
        
        driver.implicitly_wait(5) # seconds
        driver.get('http://192.168.122.89:3000/')
        driver.implicitly_wait(3) # seconds
        blog_name = driver.find_element_by_id('setting_blog_name')
        blog_name.send_keys(u'OSv blog')
        email = driver.find_element_by_id('setting_email')
        email.send_keys(u'syuu@cloudius-systems.com')
        button = driver.find_element_by_id('submit')
        button.click()
        
        driver.implicitly_wait(3) # seconds
        driver.find_element_by_link_text(u'admin').click()
        
        driver.implicitly_wait(3) # seconds
        driver.find_element_by_link_text(u'Articles').click()
        
        driver.implicitly_wait(3) # seconds
        driver.find_element_by_link_text(u'New Article').click()
        
        # Submit articles
        for i in range(1, 20):
            driver.implicitly_wait(3) # seconds
            body = driver.find_element_by_id('article_body_and_extended')
            body.send_keys(u'abcdef12345')
            title = driver.find_element_by_id('article_title')
            title.send_keys(u'article%d' % i)
            title.submit()
        
            driver.find_element_by_link_text(u'New article').click()
        
        # Get submitted articles
        driver.get('http://192.168.122.89:3000/')
        
        driver.implicitly_wait(3) # seconds
        driver.find_element_by_link_text(u'Last').click()
        
        driver.implicitly_wait(3) # seconds
        driver.find_element_by_link_text(u'Hello World!').click()
        driver.back()
        
        for i in range(1, 10):
            driver.implicitly_wait(3) # seconds
            driver.find_element_by_link_text(u'article%d' % i).click()
            driver.back()
        
        driver.get('http://192.168.122.89:3000/')
        
        for i in range(10, 20):
            driver.implicitly_wait(3) # seconds
            driver.find_element_by_link_text(u'article%d' % i).click()
            driver.back()

        return True
    except Exception, e:
        print(e)
        return False

qemu = subprocess.Popen("./scripts/run.py -n", shell=True, cwd="../..", stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
while True:
    line = qemu.stdout.readline().strip()
    print(line)

    if re.search(r'Listening on 0.0.0.0:3000', line) is not None:
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

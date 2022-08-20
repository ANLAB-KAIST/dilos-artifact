#!/usr/bin/env python3

import re
import sys
import operator
import xml.etree.ElementTree as ET
from optparse import OptionParser
import datetime

def text_to_nanos(text):
    if text.endswith('ms'):
        return float(text.rstrip('ms')) * 1e6
    if text.endswith('us'):
        return float(text.rstrip('us')) * 1e3
    if text.endswith('ns'):
        return float(text.rstrip('ns'))
    if text.endswith('s'):
        return float(text.rstrip('s')) * 1e9
    return float(text)

class wrk_output:
    pattern = r"""Running (?P<test_duration>.+?) test @ (?P<url>.+)
  (?P<nr_threads>\d+) threads and (?P<nr_connections>\d+) connections\s*
  Thread Stats   Avg      Stdev     Max   \+/- Stdev
    Latency\s+([^ ]+)\s+([^ ]+)\s+(?P<latency_max>[^ ]+)\s+([^ ]+?)
    Req/Sec\s+([^ ]+)\s+([^ ]+)\s+([^ ]+)\s+([^ ]+?)(
  Latency Distribution
     50%\s*(?P<latency_p50>.*?)
     75%\s*(?P<latency_p75>.*?)
     90%\s*(?P<latency_p90>.*?)
     99%\s*(?P<latency_p99>.*?))?
  (?P<total_requests>\d+) requests in (?P<total_duration>.+?), (?P<total_read>.+?) read(
  Socket errors: connect (?P<err_connect>\d+), read (?P<err_read>\d+), write (?P<err_write>\d+), timeout (?P<err_timeout>\d+))?(
  Non-2xx or 3xx responses: (?P<bad_responses>\d+))?
Requests/sec\:\s*(?P<req_per_sec>.+?)
Transfer/sec\:\s*(?P<transfer>.*?)\s*"""

    def __init__(self, text):
        self.m = re.match(self.pattern, text, re.MULTILINE)
        if not self.m:
            raise Exception('Input does not match')

    @property
    def requests_per_second(self):
        return self.m.group('req_per_sec')

    @property
    def error_count(self):
        return sum(map(int, [
            self.m.group('err_timeout') or '0',
            self.m.group('err_connect') or '0',
            self.m.group('err_write') or '0',
            self.m.group('err_read') or '0',
            self.m.group('bad_responses') or '0'
        ]))

    @property
    def latency_max(self):
        return text_to_nanos(self.m.group('latency_max'))

def add_time(parent, name):
    e = ET.SubElement(parent, name)
    t = datetime.datetime.utcnow()
    ET.SubElement(e, 'date', val = t.date().isoformat(), format = 'ISO8601')
    ET.SubElement(e, 'time', val = t.time().isoformat(), format = 'ISO8601')

def write_xml(w, result):
    report = ET.Element('report', categ = 'tomcat')
    add_time(report, 'start')
    test = ET.SubElement(report, 'test',
                         name = 'tomcat-wrk-perf',
                         executed = 'yes')
    ET.SubElement(test, 'description').text = (
        'Tomcat/WRK performance measurement')
    res = ET.SubElement(test, 'result')
    ET.SubElement(res, 'success', passed = 'yes',
                  state = '1', hasTimeOut = 'no')
    ET.SubElement(res, 'performance', unit = 'Hz',
                  mesure = result, isRelevant = 'true')
    add_time(report, 'end')

    w.write(str(ET.tostring(report), 'UTF8'))

def print_table(data):
    formats = []

    for header, value in data:
        formats.append('%%%ds' % (max(len(str(value)), len(header))))

    format = ' '.join(formats)

    print(format % tuple(map(operator.itemgetter(0), data)))
    print(format % tuple(map(str, list(map(operator.itemgetter(1), data)))))

if __name__ == "__main__":
    op = OptionParser()
    op.add_option('-o', '--output', metavar = 'FILE',
              help = 'write output to FILE')
    op.add_option('-m', '--metric', metavar='NAME',
              help = 'name metric as NAME')

    options, args = op.parse_args()

    with open(args[0]) as file:
        summary = wrk_output(file.read())

        w = sys.stdout
        if options.output:
            w = open(options.output, 'w')

        write_xml(w, summary.requests_per_second)

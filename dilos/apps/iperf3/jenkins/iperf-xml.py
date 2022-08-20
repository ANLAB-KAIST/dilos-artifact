#!/usr/bin/python3

import xml.etree.ElementTree as ET
from optparse import OptionParser
import sys
import datetime

op = OptionParser()
op.add_option('-o', '--output', metavar = 'FILE',
              help = 'write output to FILE')
op.add_option('-m', '--metric', metavar='NAME',
              help = 'name metric as NAME')

options, args = op.parse_args()

def iter_results(lines):
    i = iter(lines)
    while True:
        line = next(i)
        if line.startswith('[ ID]'):
            line = next(i)
            print('processing {}\n'.format(line))
            yield line[5:].split()[4]

with open(args[0]) as f:
    results = list(iter_results(f))
    if not results:
        print('No results found')
        sys.exit(1)
    result = results[0]

def add_time(parent, name):
    e = ET.SubElement(parent, name)
    t = datetime.datetime.utcnow()
    ET.SubElement(e, 'date', val = t.date().isoformat(), format = 'ISO8601')
    ET.SubElement(e, 'time', val = t.time().isoformat(), format = 'ISO8601')

report = ET.Element('report', categ = 'iperf')
add_time(report, 'start')
test = ET.SubElement(report, 'test', name = 'iperf-tcp-4cpu-1s-stream', executed = 'yes')
ET.SubElement(test, 'description').text = 'iperf single stream bandwidth to 4 cpu guest'
res = ET.SubElement(test, 'result')
ET.SubElement(res, 'success', passed = 'yes', state = '1', hasTimeOut = 'no')
ET.SubElement(res, 'performance', unit = 'Mbps', mesure = result, isRelevant = 'true')
add_time(report, 'end')

w = sys.stdout
if options.output:
    w = open(options.output, 'w')

w.write(str(ET.tostring(report), 'UTF8'))
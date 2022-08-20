#!/usr/bin/python3

import xml.etree.ElementTree as ET
from optparse import OptionParser
import sys
import datetime
import re

op = OptionParser()
op.add_option('-o', '--output', metavar = 'FILE',
              help = 'write output to FILE')
op.add_option('-m', '--metric', metavar='NAME',
              help = 'name metric as NAME')

options, args = op.parse_args()

tests = {
    'MIGRATED TCP STREAM TEST': (6, 4),
    'MIGRATED TCP MAERTS TEST': (6, 4),
    'MIGRATED TCP REQUEST/RESPONSE TEST': (6, 5),
    'MIGRATED UDP REQUEST/RESPONSE TEST': (6, 5),
}

result = None
with open(args[0]) as f:
    lines = f.readlines()
    ignoreline = re.compile(r'catcher: timer popped with times_up != 0')
    lines = [x
             for x in lines
             if not re.match(ignoreline, x)]
    for name in tests:
        if lines[0].startswith(name):
            row, col = tests[name]
            result = lines[row].split()[col]
    if result is None:
        print('Cannot parse test from {}\n'.format(line));
        sys.exit(1)

def add_time(parent, name):
    e = ET.SubElement(parent, name)
    t = datetime.datetime.utcnow()
    ET.SubElement(e, 'date', val = t.date().isoformat(), format = 'ISO8601')
    ET.SubElement(e, 'time', val = t.time().isoformat(), format = 'ISO8601')

report = ET.Element('report', categ = 'netperf')
add_time(report, 'start')
test = ET.SubElement(report, 'test', name = 'netperf', executed = 'yes')
ET.SubElement(test, 'description').text = 'netperf'
res = ET.SubElement(test, 'result')
ET.SubElement(res, 'success', passed = 'yes', state = '1', hasTimeOut = 'no')
ET.SubElement(res, 'performance', unit = 'Mbps', mesure = result, isRelevant = 'true')
add_time(report, 'end')

w = sys.stdout
if options.output:
    w = open(options.output, 'w')

w.write(str(ET.tostring(report), 'UTF8'))



#!/bin/python3

from xml.etree import ElementTree as et
import sys

# We need at least one input and one output file path
if len(sys.argv) < 3:
    print('Usage: OUTPUT_FILE INPUT_FILE..')

    sys.exit(-1)

u = et.fromstring('<node></node>')
for x in sys.argv[2:]:
    d = et.parse(x)
    u.extend(d.getroot())

f = open(sys.argv[1], 'w')
f.write(et.tostring(u, encoding="unicode"))
f.close()

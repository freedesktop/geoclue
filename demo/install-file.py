#!/bin/python3

# A simple script that copies given file (first arg) to a given location (second arg)

import sys
from shutil import copyfile

if len(sys.argv) < 3:
    print('Usage: ' + sys.argv[0] + ' SOURCE_PATH DESTINATION_PATH')

    sys.exit(-1)

copyfile(sys.argv[1], sys.argv[2])

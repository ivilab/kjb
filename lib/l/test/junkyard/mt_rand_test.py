"""
This script test whether mt_rand_test.c works properly.
Requires Python 2.7
"""
import re
import sys
import subprocess

output = subprocess.check_output(["./mt_rand_test"])

dict = {}
for line in output.split('\n'):
    line = line.strip()
    if re.match('[0-9a-f]+: 0[.]\\d+', line):
        cols = line.split(':')
        k, v = cols[0].strip(), cols[1].strip()
        
        if k in dict:
            dict[k].append(v)
        else:
            dict[k] = [v]

print 'Tested agains %d threads with %d iternations' % (len(dict), len(dict.values()[0]))

# This is a naive way to get the first element from 'dict'.
first_line = dict.values()[0]

failed = False
for k in dict:
    if dict[k] != first_line:
        sys.stderr.write('Expected result = %s\n' % first_line)
        sys.stderr.write('Actual result = %s\n' % dict[k])
        failed = True
    # print '%08x: %s' % (int(k, 16), dict[k])

if not failed:
    print 'Done+'

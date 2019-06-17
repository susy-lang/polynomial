#!/usr/bin/python
#
# This script reads C++ source files and writes all
# multi-line strings into individual files.
# This can be used to extract the Polynomial test cases
# into files for e.g. fuzz testing as
# scripts/isolateTests.py tests/libpolynomial/PolynomialEndToEndTest.cpp

import sys
lines = sys.stdin.read().split('\n')
inside = False
tests = []
for l in lines:
  if inside:
    if l.strip().endswith(')";'):
      inside = False
    else:
      tests[-1] += l + '\n'
  else:
    if l.strip().endswith('R"('):
      inside = True
      tests += ['']
for i in range(len(tests)):
  open('test%d.pol' % i, 'w').write(tests[i])

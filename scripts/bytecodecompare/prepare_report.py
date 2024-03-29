#!/usr/bin/env python

import sys
import glob
import subprocess
import json

polc = sys.argv[1]
report = open("report.txt", "wb")

for optimize in [False, True]:
    for f in sorted(glob.glob("*.pol")):
        sources = {}
        sources[f] = {'content': open(f, 'r').read()}
        input = {
            'language': 'Polynomial',
            'sources': sources,
            'settings': {
                'optimizer': {
                    'enabled': optimize
                },
                'outputSelection': { '*': { '*': ['svm.bytecode.object', 'metadata'] } }
            }
        }
        args = [polc, '--standard-json']
        if optimize:
            args += ['--optimize']
        proc = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (out, err) = proc.communicate(json.dumps(input))
        try:
            result = json.loads(out.strip())
            for filename in sorted(result['contracts'].keys()):
                for contractName in sorted(result['contracts'][filename].keys()):
                    contractData = result['contracts'][filename][contractName]
                    if 'svm' in contractData and 'bytecode' in contractData['svm']:
                        report.write(filename + ':' + contractName + ' ' + contractData['svm']['bytecode']['object'] + '\n')
                    else:
                        report.write(filename + ':' + contractName + ' NO BYTECODE\n')
                    report.write(filename + ':' + contractName + ' ' + contractData['metadata'] + '\n')
        except KeyError:
            report.write(f + ": ERROR\n")

#! /usr/bin/python
import sys

filepath = sys.argv[1]
okStrings = ['[', ']', 'node', 'Latitude', 'Longitude', 'id', 'label', 'source', 'target']
with open(filepath) as fp:
	for line in fp:
		for okStr in okStrings:
			if okStr in line:
				if 'Longitude' in line:
					print '    graphics'
					print '    ['
					line = line.replace('Longitude', ' x')
					print line,
				elif 'Latitude' in line:
					line = line.replace('Latitude', ' y')
					print line,
					print '    ]'
				else:
					print line,
				break

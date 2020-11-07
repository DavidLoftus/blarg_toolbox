#!/usr/bin/env python3

import sys
import re

paths = sys.argv[1:]

data = {}

coords = set()

sequence_ids = set()

for path in paths:
    with open(path) as f:
        file_coords = set()
        for line in f:
            m = re.match(r"Data recovered: SEQ-(\d+) = '(.*)'", line)
            if m:
                seq, c = m.groups()
                seq = int(seq)
                sequence_ids.add(seq)
                continue

            m = re.match(r'Scanning (\d+),(\d+)', line)
            if m:
                x,y = map(int, m.groups())
                file_coords.add((x,y))
                coords.add((x,y))
                continue
        if not seq in data:
            data[seq] = { 'c': c, 'points': file_coords }
        else:
            data[seq]['points'] |= file_coords

print(len(coords), 'coords found.')

with open('points.txt', 'w') as f:
    for x,y in sorted(coords):
        f.write(f'{x},{y}\n')

# print(data)

A = [ [ set(sequence_ids) for j in range(44) ] for i in range(48) ]

for k,v in data.items():
    for x,y in v['points']:
        A[x][y].remove(k)

for row in A:
    for col in row:
        n = len(col)
        s = ' '*3 if n == len(sequence_ids) else '%3d' % n
        sys.stdout.write(s)
    sys.stdout.write('\n')
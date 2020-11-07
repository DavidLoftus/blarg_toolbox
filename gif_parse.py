#!/usr/bin/env python3

from PIL import Image
import math
from functools import reduce
import signal
import sys
import sys
import hashlib
import os.path
from multiprocessing.pool import ThreadPool

paths = sys.argv[1:]

n = 100

def parse_gif(path):
    gif: Image
    gif = Image.open(path)

    gif.seek(1)

    bits = [0]
    delays = [0]

    try:
        i = 0
        while True:
            bit = gif.getpixel((0,0))

            if gif.info['duration'] % 400 != 0:
                print(path, i, gif.info['duration'])

            delay = gif.info['duration']

            if bits and bits[-1] == bit:
                delays[-1] += delay
            else:
                bits.append(bit)
                delays.append(delay)

            gif.seek(gif.tell()+1)
    except EOFError:
        pass # end of sequence

    delay_hash = hashlib.md5(str(delays).encode('utf8')).hexdigest()

    return os.path.basename(path), delay_hash

results = ThreadPool(4).imap_unordered(parse_gif, paths)

hashes = {}

n = 0
try:
    for path, hsh in results:
        if not hsh in hashes:
            hashes[hsh] = []
        hashes[hsh].append(path)
        print(hsh, len(hashes[hsh]))
        n += 1
except KeyboardInterrupt:
    print(f'sigint, terminating early f{n / len(paths)}')
    pass

with open('duplicate_gifs.txt', 'w') as f:
    for hsh, paths in hashes.items():
        f.write(f'{hsh} {" ".join(paths)}\n')
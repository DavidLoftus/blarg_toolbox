#!/usr/bin/env python3

from PIL import Image
import numpy as np

image = Image.new('1', (48, 44))

with open('points.txt') as f:
    for line in f:
        x,y = map(int,line.split(','))
        image.putpixel((x,y), 0xffffff)

image.save('points.png')
import hashlib
from functools import partial
import os.path
import json

def md5sum(filename):
    with open(filename, mode='rb') as f:
        d = hashlib.md5()
        for buf in iter(partial(f.read, 128), b''):
            d.update(buf)
    return d.hexdigest()

fragments = [os.path.basename(path) for path in os.listdir('fragments/')]

hashDict = {md5sum(f'tracks/{fragment}.dat'): fragment for fragment in fragments}

pixel_coords = json.load(open('nevToolExport.json'))



while True:

    missing_pixels = set(fragments)

    for fragment, pos in pixel_coords.items():
        missing_pixels.remove(fragment)
    
    print(len(missing_pixels))
    old_len = len(missing_pixels)

    new_pixels = set()

    for fragment in missing_pixels:
        hashcode = md5sum(f'tracks/{fragment}.dat')

        with open(f'neighbours/{hashcode}.dat') as f:
            for line in f:
                score, _, track_name = line.split()
                neighbourPath = os.path.basename(track_name)
                neighbourHash = neighbourPath[0:len(neighbourPath)-len('.dat')]
                neighbour = hashDict[neighbourHash]

                if neighbour in pixel_coords:
                    found = True
                    print(fragment, "found neighbour", neighbour)
                    pixel_coords[fragment] = pixel_coords[neighbour]
                    new_pixels.add(fragment)
    
    missing_pixels -= new_pixels
    
    if old_len == len(missing_pixels):
        print(len(missing_pixels))
        break

json.dump(pixel_coords, open('my_points.json', 'w'))
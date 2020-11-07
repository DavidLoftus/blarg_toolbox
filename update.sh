#!/usr/bin/env bash

python3 form_parse.py

# for file in fragments/fragment_*.gif
# do
#     sed -n '/ERROR: TIMING/,/Fragment/p' < $file | tr -cd '\11\12\40-\176' > comments/$(basename $file).txt
# done

# python3 comment_parse.py comments/*.txt
# python3 plot.py

gifparse/build/gif_extract ./fragments/fragment_*.gif

gifparse/build/frame_build ./tracks/fragment_*.gif.dat
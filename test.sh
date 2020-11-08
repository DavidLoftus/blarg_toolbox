#!/usr/bin/env bash

for file in tracks/fragment_*.dat
do
    cp $file unique_tracks/$(md5sum $file | awk '{ print $1 }').dat
done
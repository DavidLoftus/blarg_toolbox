#!/usr/bin/env bash

mkdir -p fragments/ comments/ tracks/

mkdir -p gifparse/build

pushd gifparse/build
    cmake ../ -DCMAKE_BUILD_TYPE=Release
    cmake --build .
popd

./update.sh
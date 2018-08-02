#!/bin/bash

if [ -d build ]; then
  	rm -rf build
fi
mkdir build

cd build
cmake -DEMULATOR=1 ..
cmake --build .
cd ..

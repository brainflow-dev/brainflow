#!/bin/bash

if [ ! -d "$DIRECTORY" ]; then
  mkdir build
fi

cd build
cmake  ..
cmake --build .
cd ..

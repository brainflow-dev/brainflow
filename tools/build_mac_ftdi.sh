#!/bin/bash

if [ -d build_mac ]; then
    rm -rf build_mac
fi
mkdir build_mac
cd build_mac

cmake -DCMAKE_INSTALL_PREFIX=../installed_mac -DCMAKE_BUILD_TYPE=Release -DUSE_LIBFTDI=ON ..
nice -n 10 make -j
make install
cd ..

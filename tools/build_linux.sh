#!/bin/bash

if [ -d build_lin ]; then
    rm -rf build_lin
fi
mkdir build_lin
cd build_lin

cmake -DCMAKE_INSTALL_PREFIX=../installed_linux -DCMAKE_BUILD_TYPE=Release ..
make -j
make install
cd ..

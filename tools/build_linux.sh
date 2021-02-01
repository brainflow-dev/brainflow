#!/bin/bash

if [ -d build_lin ]; then
    rm -rf build_lin
fi
mkdir build_lin
cd build_lin

cmake -DCMAKE_INSTALL_PREFIX=../installed_linux -DWARNINGS_AS_ERRORS=ON -DCMAKE_BUILD_TYPE=Debug ..
nice -n 10 make -j
make install
cd ..

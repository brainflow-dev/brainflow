#!/bin/bash

if [ -d build ]; then
  	rm -rf build
fi
mkdir build
cd build

if [[ "$@" == "EMULATOR" ]]
then
	echo "Build emulator mode"
    cmake -DEMULATOR=1 ..
else
	echo "Build plain"
    cmake ..
fi

cmake --build .
cd ..

cp ./compiled/libBoardController.so ./python/brainflow/lib/libBoardController.so
#!/bin/bash

PROJECT_ROOT=$(realpath $(dirname `realpath $0`)/..)
BUILD_PATH=$PROJECT_ROOT/build_test

# Parse the received commands
while :; do
    case $1 in
        -c|--clean) FLAG_CLEAN="SET"            
        ;;
        *) break
    esac
    shift
done

# Cleanup the existing files
if [[ ! -z "$FLAG_CLEAN" ]]; then
    rm -rf $BUILD_PATH
fi

# Compile!
mkdir -p $BUILD_PATH
cd $BUILD_PATH
cmake -H$PROJECT_ROOT/test -DCMAKE_BUILD_TYPE=Debug # -DSIMPLEDBUS_COVERAGE=On
make -j7

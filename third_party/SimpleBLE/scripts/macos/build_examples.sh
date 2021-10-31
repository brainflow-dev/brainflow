#!/bin/bash

realpath() {
  OURPWD=$PWD
  cd "$(dirname "$1")"
  LINK=$(readlink "$(basename "$1")")
  while [ "$LINK" ]; do
    cd "$(dirname "$LINK")"
    LINK=$(readlink "$(basename "$1")")
  done
  REALPATH="$PWD/$(basename "$1")"
  cd "$OURPWD"
  echo "$REALPATH"
}

PROJECT_ROOT=$(realpath $(dirname `realpath $0`)/../..)
SOURCE_ROOT=$PROJECT_ROOT/examples
BUILD_PATH=$PROJECT_ROOT/build

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
cmake -H$SOURCE_ROOT
make -j1

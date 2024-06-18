#!/bin/bash

# Some portions of this file where inspired from:
#   https://medium.com/@Drew_Stokes/bash-argument-parsing-54f3b81a6a8f

# If the current script is running in MacOS, print a warning
if [[ "$OSTYPE" == "darwin"* ]]; then

    # Define the realpath function, as MacOs doesn't have it
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

fi

# Parse incoming arguments
PARAMS=""
while (( "$#" )); do
  case "$1" in
    -c|--clean)
        FLAG_CLEAN=0
        shift
        ;;
    -p|--plain)
        FLAG_PLAIN=0
        shift
        ;;
    -i|--install)
        FLAG_INSTALL=0
        shift
        ;;
    -*|--*=) # unsupported flags
        echo "Error: Unsupported flag $1" >&2
        exit 1
        ;;
    *) # preserve positional arguments
        PARAMS="$PARAMS $1"
        shift
        ;;
  esac
done

# Set positional arguments in their proper place
eval set -- "$PARAMS"

PROJECT_ROOT=$(realpath $(dirname `realpath $0`)/..)
SOURCE_PATH=$PROJECT_ROOT/simplepyble
BUILD_PATH=$PROJECT_ROOT/build_simplepyble # Note: setup.py will append the project name to the build path
DIST_PATH=$BUILD_PATH/dist

# If FLAG_CLEAN is set, clean the build directory
if [[ ! -z "$FLAG_CLEAN" ]]; then
    rm -rf "$BUILD_PATH"_simplepyble
fi

if [[ ! -z "$FLAG_PLAIN" ]]; then
    BUILD_PLAIN="--plain"
fi

SETUP_INSTRUCTIONS="build --build-base $BUILD_PATH"
SETUP_INSTRUCTIONS="$SETUP_INSTRUCTIONS egg_info --egg-base $BUILD_PATH"
SETUP_INSTRUCTIONS="$SETUP_INSTRUCTIONS bdist_wheel --dist-dir $DIST_PATH"

cd $SOURCE_PATH
python3 setup.py $SETUP_INSTRUCTIONS $BUILD_PLAIN  
cd -

if [[ ! -z "$FLAG_INSTALL" ]]; then
    pip3 install $DIST_PATH/*.whl --force-reinstall
fi
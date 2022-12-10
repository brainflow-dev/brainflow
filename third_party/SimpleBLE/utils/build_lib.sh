#!/usr/bin/env bash

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
    -s|--shared)
        FLAG_SHARED=0
        shift
        ;;
    -t|--test)
        FLAG_TEST=0
        shift
        ;;
    -e|--examples)
        FLAG_EXAMPLE=0
        shift
        ;;
    -l|--local)
        FLAG_LOCAL=0
        shift
        ;;
    -p|--plain)
        FLAG_PLAIN=0
        shift
        ;;
    -sa|--sanitize_address)
        FLAG_SANITIZE_ADDRESS=0
        shift
        ;;
    -st|--sanitize_thread)
        FLAG_SANITIZE_THREAD=0
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

# Don't allow sanitize flags to coexist
if [ ! -z "$FLAG_SANITIZE_ADDRESS" ] && [ ! -z "$FLAG_SANITIZE_THREAD" ]; then
    echo "Error: Cannot use both --sanitize_address and --sanitize_thread" >&2
    exit 1
fi

# Set positional arguments in their proper place
eval set -- "$PARAMS"

# Validate that there is at least one positional argument
if [ -z "$1" ]; then
    echo "Error: No library specified" >&2
    exit 1
fi
LIB_NAME=$1

# Extract extra build arguments from the positional arguments
EXTRA_BUILD_ARGS=${@:2}

PROJECT_ROOT=$(realpath $(dirname `realpath $0`)/..)
SOURCE_PATH=$PROJECT_ROOT/$LIB_NAME
BUILD_PATH=$PROJECT_ROOT/build_$LIB_NAME
INSTALL_PATH=$BUILD_PATH/install

EXAMPLE_BUILD_PATH=$PROJECT_ROOT/build_"$LIB_NAME"_examples
EXAMPLE_SOURCE_PATH=$PROJECT_ROOT/examples/$LIB_NAME

# If FLAG_TEST is set, build the library with the test argument
if [[ ! -z "$FLAG_TEST" ]]; then
    BUILD_TEST_ARG="-D${LIB_NAME^^}_TEST=ON"

    # If FLAG_SANITIZE_ADDRESS is set, build the library with the sanitize address argument
    if [[ ! -z "$FLAG_SANITIZE_ADDRESS" ]]; then
        BUILD_SANITIZE_ADDRESS_ARG="-D${LIB_NAME^^}_SANITIZE=Address"
    fi

    # If FLAG_SANITIZE_THREAD is set, build the library with the sanitize thread argument
    if [[ ! -z "$FLAG_SANITIZE_THREAD" ]]; then
        BUILD_SANITIZE_THREAD_ARG="-D${LIB_NAME^^}_SANITIZE=Thread"
    fi

fi

if [[ ! -z "$FLAG_SHARED" ]]; then
    BUILD_SHARED_ARG="-DBUILD_SHARED_LIBS=ON"
fi

if [[ ! -z "$FLAG_PLAIN" ]]; then
    BUILD_PLAIN="-DSIMPLEBLE_PLAIN=ON"
fi

# If FLAG_CLEAN is set, clean the build directory
if [[ ! -z "$FLAG_CLEAN" ]]; then
    rm -rf $BUILD_PATH
    rm -rf $EXAMPLE_BUILD_PATH
fi

cmake -H$SOURCE_PATH -B $BUILD_PATH $BUILD_TEST_ARG $BUILD_SANITIZE_ADDRESS_ARG $BUILD_SANITIZE_THREAD_ARG $BUILD_SHARED_ARG $BUILD_PLAIN $EXTRA_BUILD_ARGS
cmake --build $BUILD_PATH -j7
cmake --install $BUILD_PATH --prefix "${INSTALL_PATH}"

# If FLAG_LOCAL is set, we want to build examples out of source files instead of the installed library
if [[ ! -z "$FLAG_LOCAL" ]]; then
    BUILD_EXAMPLE_ARGS="-D${LIB_NAME^^}_LOCAL=ON"
else
    BUILD_EXAMPLE_ARGS="-D${LIB_NAME^^}_LOCAL=OFF -D${LIB_NAME}_ROOT=${INSTALL_PATH}"
fi

if [[ ! -z "$FLAG_EXAMPLE" ]]; then
    cmake -H$EXAMPLE_SOURCE_PATH -B $EXAMPLE_BUILD_PATH $BUILD_EXAMPLE_ARGS $BUILD_SHARED_ARG
    cmake --build $EXAMPLE_BUILD_PATH -j7
fi

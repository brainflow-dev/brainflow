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
    -e|--examples)
        FLAG_EXAMPLE=0
        shift
        ;;
    -d|--deploy)
        FLAG_DEPLOY=0
        shift
        ;;
    -r|--run)
        FLAG_RUN=$2
        shift
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

# Base path definitions
PROJECT_ROOT=$(realpath $(dirname `realpath $0`)/..)
SOURCE_PATH=$PROJECT_ROOT/simpleble
BUILD_PATH=$PROJECT_ROOT/build_simpleble_android
INSTALL_PATH=$BUILD_PATH/install

EXAMPLE_BUILD_PATH=$PROJECT_ROOT/build_simpleble_android_examples
EXAMPLE_SOURCE_PATH=$PROJECT_ROOT/examples/simpleble

# If FLAG_CLEAN is set, clean the build directory
if [[ ! -z "$FLAG_CLEAN" ]]; then
    rm -rf $BUILD_PATH
    rm -rf $EXAMPLE_BUILD_PATH
fi

# Check if the ANDROID_NDK_HOME environment variable is set
if [ -z "$ANDROID_NDK_HOME" ]; then
    echo "The ANDROID_NDK_HOME environment variable is not set. Please set it to the path of your Android NDK installation."
    exit 1
fi

# These are some hardcoded variables used for my test process. You can change them to fit your needs.
ANDROID_ARCH_ABI="armeabi-v7a"
ANDROID_API=21
# NOTE: Also look at ANDROID_STL_TYPE
ANDROID_ARGS="-DCMAKE_SYSTEM_NAME=Android -DCMAKE_ANDROID_NDK=$ANDROID_NDK_HOME -DCMAKE_ANDROID_ARCH_ABI=$ANDROID_ARCH_ABI -DCMAKE_ANDROID_API=$ANDROID_API"

cmake -H$SOURCE_PATH -B $BUILD_PATH $ANDROID_ARGS
cmake --build $BUILD_PATH -j7
cmake --install $BUILD_PATH --prefix "${INSTALL_PATH}"

if [[ ! -z "$FLAG_EXAMPLE" ]]; then
    cmake -H$EXAMPLE_SOURCE_PATH -B $EXAMPLE_BUILD_PATH $ANDROID_ARGS
    cmake --build $EXAMPLE_BUILD_PATH -j7

    if [[ ! -z "$FLAG_DEPLOY" ]]; then
        adb shell rm -rf /data/local/tmp/simpleble
        adb shell mkdir /data/local/tmp/simpleble
        adb push $EXAMPLE_BUILD_PATH/bin/* /data/local/tmp/simpleble
        adb shell chmod +x /data/local/tmp/simpleble/*
    fi

    if [[ ! -z "$FLAG_RUN" ]]; then
        adb shell /data/local/tmp/simpleble/$FLAG_RUN
    fi
fi
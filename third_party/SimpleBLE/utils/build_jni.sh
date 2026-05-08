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
    -d|--debug)
        FLAG_DEBUG=0
        shift
        ;;
    -e|--examples)
        FLAG_EXAMPLES=0
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

# Extract extra build arguments from the positional arguments
EXTRA_BUILD_ARGS=${@:1}

PROJECT_ROOT=$(realpath $(dirname `realpath $0`)/..)

GRADLE_HOME=$PROJECT_ROOT/.gradle
GRADLE_CMD=$PROJECT_ROOT/utils/gradle/gradlew
GRADLE_ACTIONS=""

SOURCE_PATH=$PROJECT_ROOT/simplejavable/java
BUILD_PATH=$PROJECT_ROOT/build_simplejavable
EXAMPLES_PATH=$PROJECT_ROOT/examples/simplejavable
EXAMPLES_BUILD_PATH=$PROJECT_ROOT/build_simplejavable_examples

# If FLAG_CLEAN is set, clean the build directory
if [[ ! -z "$FLAG_CLEAN" ]]; then
    GRADLE_ACTIONS="$GRADLE_ACTIONS clean"
fi

if [[ ! -z "$FLAG_DEBUG" ]]; then
    # TODO
    echo "Debug build not supported yet"
    exit 1
fi

if [[ ! -z "$FLAG_EXAMPLES" ]]; then
    GRADLE_ACTIONS="$GRADLE_ACTIONS buildAllJars"
    exec $GRADLE_CMD -g $GRADLE_HOME -p $EXAMPLES_PATH -PbuildFromCMake -Dorg.gradle.project.buildDir=$EXAMPLES_BUILD_PATH $GRADLE_ACTIONS
else
    GRADLE_ACTIONS="$GRADLE_ACTIONS build"
    exec $GRADLE_CMD -g $GRADLE_HOME -p $SOURCE_PATH -PbuildFromCMake -Dorg.gradle.project.buildDir=$BUILD_PATH $GRADLE_ACTIONS
fi

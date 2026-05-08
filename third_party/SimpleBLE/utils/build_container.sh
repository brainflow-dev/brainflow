#! /bin/bash

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

PROJECT_ROOT=$(realpath $(dirname `realpath $0`)/..)
SOURCE_PATH=$PROJECT_ROOT/$LIB_NAME

docker build -t simpleble-container:arm64 -f $PROJECT_ROOT/utils/containers/Dockerfile.rs.dockcross-arm64 $PROJECT_ROOT/utils/containers
docker run -it --rm -v $PROJECT_ROOT:/work/src simpleble-container:arm64 /bin/bash
#!/usr/bin/env bash

# Script to run clang-format locally and print differences using colordiff
# only for files that have changes.
#
# Dry run:
# >./format.sh 
#
# Apply changes:
# >./format.sh apply


CLANG_FORMAT="clang-format"

CLANG_FORMAT_VERSION=$($CLANG_FORMAT --version | sed 's/.*version \([1-9][0-9]*\)\..*/\1/')
if [ "$CLANG_FORMAT_VERSION" != "14" ]; then
    echo "CI/CD uses clang-format version 14. Local version is $CLANG_FORMAT_VERSION"
    exit 1
fi

# Check if colordiff is installed
if ! command -v colordiff &> /dev/null; then
    echo "colordiff is required but not installed. Please install it and run the script again."
    exit 1
fi


APPLY=false
# Check if the 'apply' argument is provided
if [ -n "$1" ]; then
    if [ "$1" == "apply" ]; then
        APPLY=true
    else
        # If the first argument is not 'apply', print usage and exit
        echo "Invalid argument. Usage:"
        echo "format.sh apply"
        exit 1
    fi
fi


if [ "$1" == "apply" ]; then
    APPLY=true
fi

# Run clang-format and print differences only for files that have changes
echo "Running clang-format..."

# Excluded paths
EXCLUDED_PATHS=(
    "./simplepyble/*"
    "*CMakeFiles*"
    "*_deps*"
    "*build*"
)

FIND_CMD="find . -type f \( -name \"*.h\" -o -name \"*.hpp\" -o -name \"*.cpp\" -o -name \"*.c\" \)"
for path in "${EXCLUDED_PATHS[@]}"; do
    FIND_CMD+=" ! -path \"$path\""
done

# Find all relevant source files, excluding specified paths, and process each file
eval "$FIND_CMD" | while read -r file; do

    # Get the differences between the original file and the formatted file
    diff_output=$($CLANG_FORMAT "$file" | diff -u "$file" - | colordiff)

    # If differences are found, print the file name and the differences
    if [ -n "$diff_output" ]; then
        if [ "$APPLY" == true ]; then
            $CLANG_FORMAT -i "$file"
            echo "Applied changes to $file"
        else
            echo "Differences found in $file:"
            echo "$diff_output"
        fi
    fi
done

echo "Format completed."
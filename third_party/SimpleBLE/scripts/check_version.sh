#!/bin/bash

validate_version_format() {
    local version_file_content=$1
    echo "- VERSION file format? ($version_file_content)"
    if [[ ! "$version_file_content" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
        echo "  FAIL, format should be major.minor.patch"
        return 1
    else
        echo "  OK"
        return 0
    fi
}

check_tag_version_match() {
    local current_commit_tag=$1
    local version_file=$2
    local last_tag=$3

    if [[ -n "$current_commit_tag" ]]; then
        # Current commit is tagged
        echo "- Git tag consistent with VERSION file?"
        if [[ "$current_commit_tag" == "$version_file" ]]; then
            echo "  OK"
            return 0
        else
            echo "  FAIL Tag ($current_commit_tag) does not match VERSION file content ($version_file)."
            return 1
        fi
    else
        # Current commit is not tagged, so VERSION file must have been increased manually
        # Lets check...
        local smallest_tag=$(printf '%s\n' "$version_file" "$last_tag" | sort -V | head -n1)
        echo "- VERSION file ($version_file) bumped after last released tag ($last_tag)?"
        if [[ "$last_tag" == "$version_file" || "$smallest_tag" == "$version_file" ]]; then
            echo "  FAIL Version in development should be greater than the last tag, did you forget to bump it?"
            return 1
        else
            echo "  OK"
            return 0
        fi
    fi
}

# Function to check if Cargo.toml version matches the VERSION file
check_cargo_version_match() {
    local version_file=$1
    # Extract the version line from Cargo.toml
    local cargo_version=$2
    echo "- Cargo.toml consistent with VERSION file?"
    if [[ "$cargo_version" == "$version_file" ]]; then
        echo "  OK"
        return 0
    else
        echo "  FAIL Cargo.toml version ($cargo_version) does not match VERSION file content ($version_file)."
        return 1
    fi
}


check_changelog() {
    local current_commit_tag=$1
    if [[ -n "$current_commit_tag" ]]; then
        echo "- Release tag found in changelog?"
        if grep -q "$current_commit_tag" docs/changelog.rst; then
            echo "  OK"
            return 0
        else
            echo "  FAIL, did you forgot to update changelog?"
            return 1
        fi
    fi
}
###############################################################################
# Test logic

assert_return_value() {
    local command="$1"
    local expected=$2
    local description="$3"

    # Run command and redirect output to /dev/null
    eval "$command" > /dev/null 2>&1
    local actual=$?

    if [ $actual -ne $expected ]; then
        echo "UNIT TEST FAILED: $command (expected: $expected, got: $actual) Check code logic"
        return 1
    fi
}

assert_return_value 'validate_version_format "1.1"' 1  || exit 1
assert_return_value 'validate_version_format "1.1.1"' 0 || exit 1
assert_return_value 'check_cargo_version_match "1.1.1" "1.1.2"' 1 || exit 1
assert_return_value 'check_cargo_version_match "1.1.1" "1.1.1"' 0 || exit 1
assert_return_value 'check_tag_version_match "1.1.1" "1.1.2" "1.1.1"' 1 || exit 1
assert_return_value 'check_tag_version_match "1.1.1" "1.1.1" "1.1.1"' 0 || exit 1
assert_return_value 'check_tag_version_match "" "1.1.1" "1.1.1"' 1 || exit 1
assert_return_value 'check_tag_version_match "" "1.1.0" "1.1.1"' 1 || exit 1
assert_return_value 'check_tag_version_match "" "1.1.2" "1.1.1"' 0 || exit 1
assert_return_value 'check_changelog "1.1.9" ' 1 || exit 1
assert_return_value 'check_changelog "0.7.0" ' 0 || exit 1
assert_return_value 'check_changelog "" ' 0 || exit 1
###############################################################################

# Perform checks with real repository versions

current_tag=$(git describe --exact-match --tags HEAD 2>/dev/null | sed 's/^v//')
latest_tag=$(git describe --tags --abbrev=0 | sed 's/^v//')
version_file=$(cat VERSION)
cargo_version=$(grep '^version = ' Cargo.toml | sed 's/version = "\(.*\)"/\1/')

validate_version_format "$version_file" || exit 1
check_cargo_version_match "$version_file" "$cargo_version" || exit 1
check_tag_version_match "$current_tag" "$version_file" "$latest_tag" || exit 1
check_changelog "$current_tag"

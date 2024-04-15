param (
    [Alias('c')]
    [switch]$clean = $false,
    [Alias('e')]
    [switch]$examples = $false,
    [validateset('x86','x64')]
    [Alias('a')]
    [string]$arch = "x64" # x86, x64
)

[string]$PROJECT_ROOT = Resolve-Path $($PSScriptRoot + "\..")
[string]$SOURCE_PATH = $PROJECT_ROOT + "\simpleble"
[string]$BUILD_PATH = $PROJECT_ROOT + "\build_simpleble"
[string]$INSTALL_PATH = $BUILD_PATH + "\install"
[string]$TARGET = "Release"

[string]$EXAMPLE_BUILD_PATH = $PROJECT_ROOT + "\build_simpleble_examples"
[string]$EXAMPLE_SOURCE_PATH = $PROJECT_ROOT + "\examples\simpleble"

# Validate the received architecture
switch -regex ($arch) {
  'x86' {
        [string]$WINDOWS_ARCH = "Win32"
    }
  'x64' {
        [string]$WINDOWS_ARCH = "x64"
    }
}

# Clean directories if needed and recreate the necessary folder structure.
if ($clean) {
    Remove-Item -Path "$BUILD_PATH" -Force -Recurse -ErrorAction SilentlyContinue
    Remove-Item -Path "$EXAMPLE_BUILD_PATH" -Force -Recurse -ErrorAction SilentlyContinue
}
New-Item -ItemType Directory -Force -Path "$BUILD_PATH" | Out-Null

# Run CMake to create our build files.
cmake -S "$SOURCE_PATH" -B "$BUILD_PATH" -DCMAKE_SYSTEM_VERSION="10.0.19041.0" -DBUILD_SHARED_LIBS=ON -A $WINDOWS_ARCH
cmake --build "$BUILD_PATH" --config $TARGET --parallel 7
cmake --install "$BUILD_PATH" --prefix "$INSTALL_PATH"

# Build the examples if needed.
if ($examples) {
    # Clean directories if needed and recreate the necessary folder structure.
    if ($clean) {
        Remove-Item -Path "$EXAMPLE_BUILD_PATH" -Force -Recurse -ErrorAction SilentlyContinue
    }
    New-Item -ItemType Directory -Force -Path "$EXAMPLE_BUILD_PATH" | Out-Null

    # Run CMake to create our build files.
    cmake -S "$EXAMPLE_SOURCE_PATH" -B "$EXAMPLE_BUILD_PATH" -DCMAKE_SYSTEM_VERSION="10.0.19041.0" -A $WINDOWS_ARCH # -DSIMPLEBLE_LOCAL=ON
    cmake --build "$EXAMPLE_BUILD_PATH" --config $TARGET --parallel 7
}
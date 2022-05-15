param (
    [Alias('c')]
    [switch]$clean = $false,
    [validateset('x86','x64')]
    [Alias('a')]
    [string]$arch = "x64" # x86, x64
)

[string]$PROJECT_ROOT = Resolve-Path $($PSScriptRoot + "\..\..")
[string]$BUILD_PATH = $PROJECT_ROOT + "\build"
[string]$SOURCE_ROOT = $PROJECT_ROOT + "\examples"
[string]$TARGET = "Release"

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
}
New-Item -ItemType Directory -Force -Path "$BUILD_PATH" | Out-Null

# Run CMake to create our build files.
cmake -S "$SOURCE_ROOT" -B "$BUILD_PATH" -A $WINDOWS_ARCH -DCMAKE_SYSTEM_VERSION="10.0.22000.0"
cmake --build "$BUILD_PATH" --config $TARGET

#Copy all generated files to the bin folder for consistency and remove the output folder.
Copy-item -Force -Recurse "$BUILD_PATH\bin\$TARGET\*" -Destination "$BUILD_PATH\bin\"
#Remove-Item -Path "$PROJECT_ROOT\bin\$TARGET" -Force -Recurse -ErrorAction SilentlyContinue
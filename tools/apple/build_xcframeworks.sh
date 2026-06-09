#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_ROOT="${ROOT_DIR}/build_apple"
OUTPUT_DIR="${ROOT_DIR}/build/apple_xcframeworks"
CONFIGURATION="Release"
BUILD_FROM_SOURCE=1
BUILD_MACOS=1
BUILD_IOS=1
BUILD_IOS_SIM=1
MACOS_PREFIX=""
IOS_PREFIX=""
IOS_SIM_PREFIX=""
BRAINFLOW_VERSION="${BRAINFLOW_VERSION:-0.0.1}"
RELEASE_BASE_URL="${BRAINFLOW_APPLE_RELEASE_BASE_URL:-}"

usage() {
  cat <<'USAGE'
Usage: tools/apple/build_xcframeworks.sh [options]

Build BrainFlow native Apple binaries and package them as framework-wrapped
XCFrameworks for standard iOS/macOS app embedding.

Options:
  --output <dir>             Output directory. Default: build/apple_xcframeworks
  --build-root <dir>         CMake build root. Default: build_apple
  --configuration <name>     CMake configuration. Default: Release
  --skip-build               Package existing install prefixes instead of building.
  --skip-macos-build         Reuse --macos-prefix and build/package the other slices.
  --skip-ios-build           Reuse --ios-prefix and build/package the other slices.
  --skip-ios-sim-build       Reuse --ios-sim-prefix and build/package the other slices.
  --macos-prefix <dir>       Existing macOS install prefix for --skip-build.
  --ios-prefix <dir>         Existing iOS device install prefix for --skip-build.
  --ios-sim-prefix <dir>     Existing iOS simulator install prefix for --skip-build.
  -h, --help                 Show this help.

Environment:
  BRAINFLOW_APPLE_BUILD_BLE=ON|OFF          Default: OFF
  BRAINFLOW_APPLE_BUILD_BLUETOOTH=ON|OFF    Default: OFF
  BRAINFLOW_APPLE_BUILD_ONNX=ON|OFF         Default: OFF
  BRAINFLOW_APPLE_RELEASE_BASE_URL=<url>     Base URL for SwiftPM release zips.

The script packages every produced dynamic library as an XCFramework. The
BrainFlow core libraries are required:
  libBoardController.dylib, libDataHandler.dylib, libMLModule.dylib
USAGE
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --output)
      OUTPUT_DIR="$2"
      shift 2
      ;;
    --build-root)
      BUILD_ROOT="$2"
      shift 2
      ;;
    --configuration)
      CONFIGURATION="$2"
      shift 2
      ;;
    --skip-build)
      BUILD_FROM_SOURCE=0
      shift
      ;;
    --skip-macos-build)
      BUILD_MACOS=0
      shift
      ;;
    --skip-ios-build)
      BUILD_IOS=0
      shift
      ;;
    --skip-ios-sim-build)
      BUILD_IOS_SIM=0
      shift
      ;;
    --macos-prefix)
      MACOS_PREFIX="$2"
      shift 2
      ;;
    --ios-prefix)
      IOS_PREFIX="$2"
      shift 2
      ;;
    --ios-sim-prefix)
      IOS_SIM_PREFIX="$2"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "error: unknown option $1" >&2
      usage >&2
      exit 2
      ;;
  esac
done

absolute_path() {
  case "$1" in
    /*) echo "$1" ;;
    *) echo "${ROOT_DIR}/$1" ;;
  esac
}

BUILD_ROOT="$(absolute_path "${BUILD_ROOT}")"
OUTPUT_DIR="$(absolute_path "${OUTPUT_DIR}")"

command -v cmake >/dev/null || { echo "error: cmake is required" >&2; exit 1; }
command -v ninja >/dev/null || { echo "error: ninja is required" >&2; exit 1; }
command -v swift >/dev/null || { echo "error: swift is required" >&2; exit 1; }
command -v xcodebuild >/dev/null || { echo "error: xcodebuild is required" >&2; exit 1; }
command -v install_name_tool >/dev/null || { echo "error: install_name_tool is required" >&2; exit 1; }
command -v vtool >/dev/null || { echo "error: vtool is required" >&2; exit 1; }

BUILD_BLE="${BRAINFLOW_APPLE_BUILD_BLE:-OFF}"
BUILD_BLUETOOTH="${BRAINFLOW_APPLE_BUILD_BLUETOOTH:-OFF}"
BUILD_ONNX="${BRAINFLOW_APPLE_BUILD_ONNX:-OFF}"

MACOS_PREFIX="${MACOS_PREFIX:-${BUILD_ROOT}/installed_macos}"
IOS_PREFIX="${IOS_PREFIX:-${BUILD_ROOT}/installed_ios}"
IOS_SIM_PREFIX="${IOS_SIM_PREFIX:-${BUILD_ROOT}/installed_ios_sim}"
MACOS_PREFIX="$(absolute_path "${MACOS_PREFIX}")"
IOS_PREFIX="$(absolute_path "${IOS_PREFIX}")"
IOS_SIM_PREFIX="$(absolute_path "${IOS_SIM_PREFIX}")"
RELEASE_BASE_URL="${RELEASE_BASE_URL:-https://github.com/brainflow-dev/brainflow/releases/download/${BRAINFLOW_VERSION}}"
RELEASE_BASE_URL="${RELEASE_BASE_URL%/}"

REQUIRED_LIBS=(
  libBoardController.dylib
  libDataHandler.dylib
  libMLModule.dylib
)

cmake_common_args=(
  -G Ninja
  "-DCMAKE_BUILD_TYPE=${CONFIGURATION}"
  "-DBRAINFLOW_VERSION=${BRAINFLOW_VERSION}"
  -DBUILD_TESTS=OFF
  -DBUILD_SYNCHRONI_SDK=OFF
  -DBUILD_PERIPHERY=OFF
  "-DBUILD_BLE=${BUILD_BLE}"
  "-DBUILD_BLUETOOTH=${BUILD_BLUETOOTH}"
  "-DBUILD_ONNX=${BUILD_ONNX}"
  -DBRAINFLOW_COPY_TO_PACKAGE_DIRS=OFF
)

sanitize_bundle_version() {
  local version="$1"
  if [[ "${version}" =~ ^[0-9]+([.][0-9]+){0,2}$ ]]; then
    echo "${version}"
  else
    echo "0.0.1"
  fi
}

FRAMEWORK_BUNDLE_SHORT_VERSION="$(sanitize_bundle_version "${BRAINFLOW_VERSION}")"
FRAMEWORK_BUNDLE_VERSION="$(sanitize_bundle_version "${BRAINFLOW_APPLE_FRAMEWORK_BUILD:-${BRAINFLOW_VERSION}}")"

build_prefix() {
  local build_dir="$1"
  local install_prefix="$2"
  shift 2

  rm -rf "${install_prefix}"
  cmake -S "${ROOT_DIR}" -B "${build_dir}" \
    "${cmake_common_args[@]}" \
    "-DCMAKE_INSTALL_PREFIX=${install_prefix}" \
    "$@"
  ninja -C "${build_dir}" clean
  ninja -C "${build_dir}" install
}

if [[ "${BUILD_FROM_SOURCE}" -eq 0 ]]; then
  BUILD_MACOS=0
  BUILD_IOS=0
  BUILD_IOS_SIM=0
fi

if [[ "${BUILD_MACOS}" -eq 1 ]]; then
  build_prefix "${BUILD_ROOT}/macos" "${MACOS_PREFIX}" \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0
fi

if [[ "${BUILD_IOS}" -eq 1 ]]; then
  build_prefix "${BUILD_ROOT}/ios" "${IOS_PREFIX}" \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_SYSROOT=iphoneos \
    -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=15.0 \
    -DBRAINFLOW_APPLE_DYNAMIC_FRAMEWORKS=ON
fi

if [[ "${BUILD_IOS_SIM}" -eq 1 ]]; then
  build_prefix "${BUILD_ROOT}/ios-simulator" "${IOS_SIM_PREFIX}" \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_SYSROOT=iphonesimulator \
    -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=15.0 \
    -DBRAINFLOW_APPLE_DYNAMIC_FRAMEWORKS=ON
fi

for prefix in "${MACOS_PREFIX}" "${IOS_PREFIX}" "${IOS_SIM_PREFIX}"; do
  if [[ ! -d "${prefix}/lib" ]]; then
    echo "error: missing install prefix library directory: ${prefix}/lib" >&2
    exit 1
  fi
done

for lib in "${REQUIRED_LIBS[@]}"; do
  for prefix in "${MACOS_PREFIX}" "${IOS_PREFIX}" "${IOS_SIM_PREFIX}"; do
    if [[ ! -f "${prefix}/lib/${lib}" ]]; then
      echo "error: missing required BrainFlow library ${prefix}/lib/${lib}" >&2
      exit 1
    fi
  done
done

rm -rf "${OUTPUT_DIR}"
mkdir -p "${OUTPUT_DIR}/XCFrameworks" "${OUTPUT_DIR}/FrameworkSlices" "${OUTPUT_DIR}/BrainFlowSwiftBinaryPackage"

framework_name_for_lib() {
  local lib_name
  lib_name="$(basename "$1")"
  lib_name="${lib_name%.dylib}"
  lib_name="${lib_name#lib}"
  echo "${lib_name}"
}

is_required_lib() {
  local lib_name="$1"
  local required
  for required in "${REQUIRED_LIBS[@]}"; do
    if [[ "${required}" == "${lib_name}" ]]; then
      return 0
    fi
  done
  return 1
}

macho_supports_platform() {
  local binary_path="$1"
  local expected_platform="$2"
  vtool -show-build "${binary_path}" 2>/dev/null | grep -q "platform ${expected_platform}"
}

bundle_identifier_for_framework() {
  local framework_name="$1"
  local identifier_name
  identifier_name="$(echo "${framework_name}" | tr '[:upper:]' '[:lower:]' | tr -c '[:alnum:]' '-')"
  identifier_name="${identifier_name%-}"
  echo "org.brainflow.${identifier_name}"
}

write_info_plist() {
  local plist_path="$1"
  local framework_name="$2"
  local platform_name="$3"
  local bundle_identifier
  bundle_identifier="$(bundle_identifier_for_framework "${framework_name}")"

  cat > "${plist_path}" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleDevelopmentRegion</key>
  <string>en</string>
  <key>CFBundleExecutable</key>
  <string>${framework_name}</string>
  <key>CFBundleIdentifier</key>
  <string>${bundle_identifier}</string>
  <key>CFBundleInfoDictionaryVersion</key>
  <string>6.0</string>
  <key>CFBundleName</key>
  <string>${framework_name}</string>
  <key>CFBundlePackageType</key>
  <string>FMWK</string>
  <key>CFBundleShortVersionString</key>
  <string>${FRAMEWORK_BUNDLE_SHORT_VERSION}</string>
  <key>CFBundleSupportedPlatforms</key>
  <array>
    <string>${platform_name}</string>
  </array>
  <key>CFBundleVersion</key>
  <string>${FRAMEWORK_BUNDLE_VERSION}</string>
</dict>
</plist>
PLIST
}

write_module_map() {
  local module_map_path="$1"
  local framework_name="$2"
  local umbrella_header="$3"

  if [[ ! "${framework_name}" =~ ^[A-Za-z_][A-Za-z0-9_]*$ ]]; then
    return
  fi

  cat > "${module_map_path}" <<MODULEMAP
framework module ${framework_name} {
  umbrella header "${umbrella_header}"
  export *
  module * { export * }
}
MODULEMAP
}

write_umbrella_header() {
  local header_path="$1"
  local framework_name="$2"

  case "${framework_name}" in
    BoardController)
      cat > "${header_path}" <<'HEADER'
#pragma once
#include "board_controller.h"
#include "board_info_getter.h"
#include "brainflow_array.h"
#include "brainflow_constants.h"
#include "brainflow_exception.h"
#include "brainflow_input_params.h"
#include "shared_export.h"
HEADER
      ;;
    DataHandler)
      cat > "${header_path}" <<'HEADER'
#pragma once
#include "brainflow_array.h"
#include "brainflow_constants.h"
#include "data_handler.h"
#include "shared_export.h"
HEADER
      ;;
    MLModule)
      cat > "${header_path}" <<'HEADER'
#pragma once
#include "brainflow_constants.h"
#include "brainflow_model_params.h"
#include "ml_module.h"
#include "shared_export.h"
HEADER
      ;;
    *)
      cat > "${header_path}" <<HEADER
#pragma once
/* ${framework_name} does not expose public BrainFlow C headers. */
HEADER
      ;;
  esac
}

copy_public_header() {
  local install_prefix="$1"
  local header_name="$2"
  local destination_dir="$3"
  local source_header="${install_prefix}/inc/${header_name}"

  if [[ ! -f "${source_header}" ]]; then
    echo "error: missing public header ${source_header}" >&2
    exit 1
  fi
  cp "${source_header}" "${destination_dir}/"
}

copy_public_headers_for_framework() {
  local install_prefix="$1"
  local framework_name="$2"
  local destination_dir="$3"
  local headers=()
  local header

  case "${framework_name}" in
    BoardController)
      headers=(
        board_controller.h
        board_info_getter.h
        brainflow_array.h
        brainflow_constants.h
        brainflow_exception.h
        brainflow_input_params.h
        shared_export.h
      )
      ;;
    DataHandler)
      headers=(
        brainflow_array.h
        brainflow_constants.h
        data_handler.h
        shared_export.h
      )
      ;;
    MLModule)
      headers=(
        brainflow_constants.h
        brainflow_model_params.h
        ml_module.h
        shared_export.h
      )
      ;;
    *)
      return
      ;;
  esac

  for header in "${headers[@]}"; do
    copy_public_header "${install_prefix}" "${header}" "${destination_dir}"
  done
}

create_framework_slice() {
  local install_prefix="$1"
  local lib_name="$2"
  local framework_name="$3"
  local slice_name="$4"
  local platform_name="$5"
  local slice_dir="${OUTPUT_DIR}/FrameworkSlices/${framework_name}/${slice_name}/${framework_name}.framework"

  rm -rf "${slice_dir}"
  mkdir -p "${slice_dir}/Headers" "${slice_dir}/Modules"

  cp "${install_prefix}/lib/${lib_name}" "${slice_dir}/${framework_name}"
  chmod 755 "${slice_dir}/${framework_name}"
  install_name_tool -id "@rpath/${framework_name}.framework/${framework_name}" "${slice_dir}/${framework_name}" || true

  copy_public_headers_for_framework "${install_prefix}" "${framework_name}" "${slice_dir}/Headers"
  write_umbrella_header "${slice_dir}/Headers/${framework_name}.h" "${framework_name}"
  write_module_map "${slice_dir}/Modules/module.modulemap" "${framework_name}" "${framework_name}.h"
  write_info_plist "${slice_dir}/Info.plist" "${framework_name}" "${platform_name}"
}

all_libs="$(
  {
    find "${MACOS_PREFIX}/lib" "${IOS_PREFIX}/lib" "${IOS_SIM_PREFIX}/lib" -maxdepth 1 -type f -name '*.dylib' -print
  } | xargs -n 1 basename | sort -u
)"

while IFS= read -r lib_name; do
  [[ -n "${lib_name}" ]] || continue

  framework_name="$(framework_name_for_lib "${lib_name}")"
  args=()

  if [[ -f "${MACOS_PREFIX}/lib/${lib_name}" ]]; then
    if macho_supports_platform "${MACOS_PREFIX}/lib/${lib_name}" MACOS; then
      create_framework_slice "${MACOS_PREFIX}" "${lib_name}" "${framework_name}" macos MacOSX
      args+=(-framework "${OUTPUT_DIR}/FrameworkSlices/${framework_name}/macos/${framework_name}.framework")
    elif is_required_lib "${lib_name}"; then
      echo "error: ${MACOS_PREFIX}/lib/${lib_name} is not a macOS Mach-O binary" >&2
      exit 1
    else
      echo "warning: skipping non-macOS optional library ${MACOS_PREFIX}/lib/${lib_name}" >&2
    fi
  fi
  if [[ -f "${IOS_PREFIX}/lib/${lib_name}" ]]; then
    if macho_supports_platform "${IOS_PREFIX}/lib/${lib_name}" IOS; then
      create_framework_slice "${IOS_PREFIX}" "${lib_name}" "${framework_name}" ios iPhoneOS
      args+=(-framework "${OUTPUT_DIR}/FrameworkSlices/${framework_name}/ios/${framework_name}.framework")
    elif is_required_lib "${lib_name}"; then
      echo "error: ${IOS_PREFIX}/lib/${lib_name} is not an iOS device Mach-O binary" >&2
      exit 1
    else
      echo "warning: skipping non-iOS optional library ${IOS_PREFIX}/lib/${lib_name}" >&2
    fi
  fi
  if [[ -f "${IOS_SIM_PREFIX}/lib/${lib_name}" ]]; then
    if macho_supports_platform "${IOS_SIM_PREFIX}/lib/${lib_name}" IOSSIMULATOR; then
      create_framework_slice "${IOS_SIM_PREFIX}" "${lib_name}" "${framework_name}" ios-simulator iPhoneSimulator
      args+=(-framework "${OUTPUT_DIR}/FrameworkSlices/${framework_name}/ios-simulator/${framework_name}.framework")
    elif is_required_lib "${lib_name}"; then
      echo "error: ${IOS_SIM_PREFIX}/lib/${lib_name} is not an iOS simulator Mach-O binary" >&2
      exit 1
    else
      echo "warning: skipping non-iOS-simulator optional library ${IOS_SIM_PREFIX}/lib/${lib_name}" >&2
    fi
  fi

  if [[ "${#args[@]}" -gt 0 ]]; then
    xcodebuild -create-xcframework "${args[@]}" -output "${OUTPUT_DIR}/XCFrameworks/${framework_name}.xcframework"
  fi
done <<< "${all_libs}"

swift_binary_package="${OUTPUT_DIR}/BrainFlowSwiftBinaryPackage"
mkdir -p "${swift_binary_package}/Sources"
cp -R "${ROOT_DIR}/swift_package/Sources/BrainFlow" "${swift_binary_package}/Sources/BrainFlow"
mkdir -p "${swift_binary_package}/XCFrameworks"
cp -R "${OUTPUT_DIR}/XCFrameworks/"*.xcframework "${swift_binary_package}/XCFrameworks/"

cat > "${swift_binary_package}/Package.swift" <<'PACKAGE'
// swift-tools-version: 5.9

import PackageDescription

let package = Package(
    name: "BrainFlow",
    platforms: [
        .macOS(.v12),
        .iOS(.v15)
    ],
    products: [
        .library(name: "BrainFlow", targets: ["BrainFlow"])
    ],
    targets: [
        .target(
            name: "BrainFlow",
            dependencies: [
                "BoardController",
                "DataHandler",
                "MLModule"
            ]
        ),
        .binaryTarget(name: "BoardController", path: "XCFrameworks/BoardController.xcframework"),
        .binaryTarget(name: "DataHandler", path: "XCFrameworks/DataHandler.xcframework"),
        .binaryTarget(name: "MLModule", path: "XCFrameworks/MLModule.xcframework")
    ]
)
PACKAGE

cat > "${swift_binary_package}/README.md" <<'README'
# BrainFlow Swift Binary Package

This package is generated by `tools/apple/build_xcframeworks.sh`.
It contains the BrainFlow Swift API and prebuilt Apple XCFrameworks for
`BoardController`, `DataHandler`, and `MLModule`.

Add this package to an iOS or macOS app through Xcode or Swift Package Manager.
Xcode embeds and signs the binary frameworks during app builds.

Do not edit generated framework contents by hand. Update BrainFlow source, rerun
the Apple artifact script, and publish the regenerated zip plus checksums and
manifest from the same build.
README

swiftpm_artifact_dir="${OUTPUT_DIR}/SwiftPMArtifacts"
remote_swift_package="${OUTPUT_DIR}/BrainFlowSwiftPackageRemote"
rm -rf "${swiftpm_artifact_dir}" "${remote_swift_package}"
mkdir -p "${swiftpm_artifact_dir}" "${remote_swift_package}/Sources"
cp -R "${ROOT_DIR}/swift_package/Sources/BrainFlow" "${remote_swift_package}/Sources/BrainFlow"

create_swiftpm_framework_zip() {
  local framework_name="$1"
  local xcframework_path="${OUTPUT_DIR}/XCFrameworks/${framework_name}.xcframework"
  local zip_path="${swiftpm_artifact_dir}/${framework_name}.xcframework.zip"

  if [[ ! -d "${xcframework_path}" ]]; then
    echo "error: missing SwiftPM XCFramework artifact ${xcframework_path}" >&2
    exit 1
  fi

  rm -f "${zip_path}"
  (
    cd "${OUTPUT_DIR}/XCFrameworks"
    /usr/bin/zip -qry "${zip_path}" "${framework_name}.xcframework"
  )
  swift package compute-checksum "${zip_path}"
}

board_controller_swiftpm_checksum="$(create_swiftpm_framework_zip BoardController)"
data_handler_swiftpm_checksum="$(create_swiftpm_framework_zip DataHandler)"
ml_module_swiftpm_checksum="$(create_swiftpm_framework_zip MLModule)"

cat > "${OUTPUT_DIR}/swiftpm-checksums.txt" <<CHECKSUMS
${board_controller_swiftpm_checksum}  SwiftPMArtifacts/BoardController.xcframework.zip
${data_handler_swiftpm_checksum}  SwiftPMArtifacts/DataHandler.xcframework.zip
${ml_module_swiftpm_checksum}  SwiftPMArtifacts/MLModule.xcframework.zip
CHECKSUMS

cat > "${OUTPUT_DIR}/swiftpm-checksums.json" <<CHECKSUMS
{
  "BoardController": {
    "artifact": "SwiftPMArtifacts/BoardController.xcframework.zip",
    "checksum": "${board_controller_swiftpm_checksum}"
  },
  "DataHandler": {
    "artifact": "SwiftPMArtifacts/DataHandler.xcframework.zip",
    "checksum": "${data_handler_swiftpm_checksum}"
  },
  "MLModule": {
    "artifact": "SwiftPMArtifacts/MLModule.xcframework.zip",
    "checksum": "${ml_module_swiftpm_checksum}"
  }
}
CHECKSUMS

cat > "${remote_swift_package}/Package.swift" <<PACKAGE
// swift-tools-version: 5.9

import PackageDescription

let package = Package(
    name: "BrainFlow",
    platforms: [
        .macOS(.v12),
        .iOS(.v15)
    ],
    products: [
        .library(name: "BrainFlow", targets: ["BrainFlow"])
    ],
    targets: [
        .target(
            name: "BrainFlow",
            dependencies: [
                "BoardController",
                "DataHandler",
                "MLModule"
            ]
        ),
        .binaryTarget(
            name: "BoardController",
            url: "${RELEASE_BASE_URL}/BoardController.xcframework.zip",
            checksum: "${board_controller_swiftpm_checksum}"
        ),
        .binaryTarget(
            name: "DataHandler",
            url: "${RELEASE_BASE_URL}/DataHandler.xcframework.zip",
            checksum: "${data_handler_swiftpm_checksum}"
        ),
        .binaryTarget(
            name: "MLModule",
            url: "${RELEASE_BASE_URL}/MLModule.xcframework.zip",
            checksum: "${ml_module_swiftpm_checksum}"
        )
    ]
)
PACKAGE

cat > "${remote_swift_package}/README.md" <<README
# BrainFlow Swift Release Package

This generated package is the release-facing SwiftPM package for BrainFlow Apple
artifacts. It uses URL-based binary targets whose ZIP checksums were generated
with \`swift package compute-checksum\`.

The generated binary target URLs use:

\`\`\`
${RELEASE_BASE_URL}
\`\`\`

Set \`BRAINFLOW_APPLE_RELEASE_BASE_URL\` before running
\`tools/apple/build_xcframeworks.sh\` when publishing release assets from a
different host, tag path, or CDN.

Publish these files next to this package version:

- \`BoardController.xcframework.zip\`
- \`DataHandler.xcframework.zip\`
- \`MLModule.xcframework.zip\`
- \`swiftpm-checksums.txt\`
- \`swiftpm-checksums.json\`
README

source_revision="unknown"
if command -v git >/dev/null && git -C "${ROOT_DIR}" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  source_revision="$(git -C "${ROOT_DIR}" rev-parse HEAD 2>/dev/null || echo unknown)"
fi

json_escape() {
  local value="$1"
  value="${value//\\/\\\\}"
  value="${value//\"/\\\"}"
  value="${value//$'\n'/ }"
  printf '%s' "${value}"
}

xcode_version="$(xcodebuild -version 2>/dev/null | tr '\n' ' ' | sed 's/[[:space:]]*$//' || true)"
cmake_version="$(cmake --version 2>/dev/null | head -n 1 | awk '{print $3}' || true)"
ninja_version="$(ninja --version 2>/dev/null || true)"

(
  cd "${OUTPUT_DIR}"
  find XCFrameworks BrainFlowSwiftBinaryPackage BrainFlowSwiftPackageRemote SwiftPMArtifacts -type f -print | LC_ALL=C sort | while IFS= read -r artifact_file; do
    shasum -a 256 "${artifact_file}"
  done > checksums.sha256
  shasum -a 256 swiftpm-checksums.txt swiftpm-checksums.json >> checksums.sha256
)

cat > "${OUTPUT_DIR}/manifest.json" <<MANIFEST
{
  "format": "brainflow-apple-xcframeworks-v1",
  "brainflow_version": "$(json_escape "${BRAINFLOW_VERSION}")",
  "configuration": "${CONFIGURATION}",
  "source_revision": "$(json_escape "${source_revision}")",
  "framework_bundle_short_version": "$(json_escape "${FRAMEWORK_BUNDLE_SHORT_VERSION}")",
  "framework_bundle_version": "$(json_escape "${FRAMEWORK_BUNDLE_VERSION}")",
  "xcode_version": "$(json_escape "${xcode_version}")",
  "cmake_version": "$(json_escape "${cmake_version}")",
  "ninja_version": "$(json_escape "${ninja_version}")",
  "build_ble": "${BUILD_BLE}",
  "build_bluetooth": "${BUILD_BLUETOOTH}",
  "build_onnx": "${BUILD_ONNX}",
  "macos_deployment_target": "12.0",
  "ios_deployment_target": "15.0",
  "swiftpm_release_base_url": "$(json_escape "${RELEASE_BASE_URL}")",
  "swiftpm_remote_package": "BrainFlowSwiftPackageRemote",
  "swiftpm_artifacts": {
    "BoardController": {
      "zip": "SwiftPMArtifacts/BoardController.xcframework.zip",
      "checksum": "$(json_escape "${board_controller_swiftpm_checksum}")"
    },
    "DataHandler": {
      "zip": "SwiftPMArtifacts/DataHandler.xcframework.zip",
      "checksum": "$(json_escape "${data_handler_swiftpm_checksum}")"
    },
    "MLModule": {
      "zip": "SwiftPMArtifacts/MLModule.xcframework.zip",
      "checksum": "$(json_escape "${ml_module_swiftpm_checksum}")"
    }
  },
  "checksums": "checksums.sha256",
  "swiftpm_checksums": "swiftpm-checksums.json"
}
MANIFEST

(
  cd "${OUTPUT_DIR}"
  /usr/bin/zip -qry BrainFlowAppleXCFrameworks.zip XCFrameworks BrainFlowSwiftBinaryPackage BrainFlowSwiftPackageRemote SwiftPMArtifacts checksums.sha256 swiftpm-checksums.txt swiftpm-checksums.json manifest.json
  shasum -a 256 BrainFlowAppleXCFrameworks.zip > BrainFlowAppleXCFrameworks.zip.sha256
)

rm -rf "${OUTPUT_DIR}/FrameworkSlices"

echo "BrainFlow Apple XCFramework artifacts written to ${OUTPUT_DIR}"
echo "Swift binary package: ${swift_binary_package}"
echo "Archive: ${OUTPUT_DIR}/BrainFlowAppleXCFrameworks.zip"

#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
ARTIFACT_DIR="${1:-${ROOT_DIR}/build/apple_xcframeworks}"
XCFRAMEWORK_DIR="${ARTIFACT_DIR}/XCFrameworks"

required_frameworks=(
  BoardController
  DataHandler
  MLModule
)

if [[ ! -d "${XCFRAMEWORK_DIR}" ]]; then
  echo "error: missing XCFramework directory: ${XCFRAMEWORK_DIR}" >&2
  exit 1
fi

command -v vtool >/dev/null || { echo "error: vtool is required" >&2; exit 1; }
command -v shasum >/dev/null || { echo "error: shasum is required" >&2; exit 1; }

macho_supports_platform() {
  local binary_path="$1"
  local expected_platform="$2"
  vtool -show-build "${binary_path}" 2>/dev/null | grep -q "platform ${expected_platform}"
}

require_platform_slice() {
  local path="$1"
  local framework="$2"
  local label="$3"
  local expected_platform="$4"
  local find_pattern="$5"
  local found=0

  while IFS= read -r binary; do
    if macho_supports_platform "${binary}" "${expected_platform}"; then
      found=1
      break
    fi
  done < <(find "${path}" -type f -path "${find_pattern}" -print)

  if [[ "${found}" -ne 1 ]]; then
    echo "error: ${framework}.xcframework is missing a ${label} Mach-O slice" >&2
    exit 1
  fi
}

for framework in "${required_frameworks[@]}"; do
  path="${XCFRAMEWORK_DIR}/${framework}.xcframework"
  if [[ ! -d "${path}" ]]; then
    echo "error: missing required XCFramework: ${path}" >&2
    exit 1
  fi

  info="${path}/Info.plist"
  available_libraries="$(/usr/libexec/PlistBuddy -c 'Print :AvailableLibraries' "${info}" 2>/dev/null || true)"
  for required_platform in macos ios; do
    if ! grep -q "SupportedPlatform = ${required_platform}" <<< "${available_libraries}"; then
      echo "error: ${framework}.xcframework is missing ${required_platform} support" >&2
      exit 1
    fi
  done

  while IFS= read -r binary; do
    [[ -n "${binary}" ]] || continue
    file "${binary}"
    if otool -D "${binary}" >/dev/null 2>&1; then
      install_name="$(otool -D "${binary}" | tail -n 1)"
      expected="@rpath/${framework}.framework/${framework}"
      if [[ "${install_name}" != "${expected}" ]]; then
        echo "error: ${binary} install name is ${install_name}, expected ${expected}" >&2
        exit 1
      fi
    fi
  done < <(find "${path}" -type f -path "*/${framework}.framework/${framework}" -print)

  require_platform_slice "${path}" "${framework}" "macOS" MACOS "*/macos-*/${framework}.framework/${framework}"
  require_platform_slice "${path}" "${framework}" "iOS device" IOS "*/ios-arm64/${framework}.framework/${framework}"
  require_platform_slice "${path}" "${framework}" "iOS simulator" IOSSIMULATOR "*/ios-*-simulator/${framework}.framework/${framework}"
done

package_dir="${ARTIFACT_DIR}/BrainFlowSwiftBinaryPackage"
if [[ ! -f "${package_dir}/Package.swift" ]]; then
  echo "error: missing generated Swift binary package: ${package_dir}" >&2
  exit 1
fi

if [[ ! -f "${ARTIFACT_DIR}/BrainFlowAppleXCFrameworks.zip" ]]; then
  echo "error: missing XCFramework archive: ${ARTIFACT_DIR}/BrainFlowAppleXCFrameworks.zip" >&2
  exit 1
fi

if [[ ! -f "${ARTIFACT_DIR}/checksums.sha256" ]]; then
  echo "error: missing artifact checksums: ${ARTIFACT_DIR}/checksums.sha256" >&2
  exit 1
fi

if [[ ! -f "${ARTIFACT_DIR}/BrainFlowAppleXCFrameworks.zip.sha256" ]]; then
  echo "error: missing archive checksum: ${ARTIFACT_DIR}/BrainFlowAppleXCFrameworks.zip.sha256" >&2
  exit 1
fi

(
  cd "${ARTIFACT_DIR}"
  shasum -a 256 -q -c checksums.sha256
  shasum -a 256 -q -c BrainFlowAppleXCFrameworks.zip.sha256
)

echo "BrainFlow Apple XCFramework verification passed: ${ARTIFACT_DIR}"

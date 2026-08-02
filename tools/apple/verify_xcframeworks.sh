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
command -v swift >/dev/null || { echo "error: swift is required" >&2; exit 1; }
command -v unzip >/dev/null || { echo "error: unzip is required" >&2; exit 1; }

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

swiftpm_artifact_dir="${ARTIFACT_DIR}/SwiftPMArtifacts"
swiftpm_checksum_file="${ARTIFACT_DIR}/swiftpm-checksums.txt"
remote_package_dir="${ARTIFACT_DIR}/BrainFlowSwiftPackageRemote"

if [[ ! -d "${swiftpm_artifact_dir}" ]]; then
  echo "error: missing SwiftPM artifact directory: ${swiftpm_artifact_dir}" >&2
  exit 1
fi

if [[ ! -f "${swiftpm_checksum_file}" ]]; then
  echo "error: missing SwiftPM checksum file: ${swiftpm_checksum_file}" >&2
  exit 1
fi

if [[ ! -f "${ARTIFACT_DIR}/swiftpm-checksums.json" ]]; then
  echo "error: missing SwiftPM checksum JSON: ${ARTIFACT_DIR}/swiftpm-checksums.json" >&2
  exit 1
fi

if [[ ! -f "${remote_package_dir}/Package.swift" ]]; then
  echo "error: missing generated remote Swift package: ${remote_package_dir}" >&2
  exit 1
fi

require_swiftpm_artifact() {
  local framework="$1"
  local zip_path="${swiftpm_artifact_dir}/${framework}.xcframework.zip"
  local artifact_path="SwiftPMArtifacts/${framework}.xcframework.zip"
  local recorded_checksum
  local computed_checksum
  local entry_count=0

  if [[ ! -f "${zip_path}" ]]; then
    echo "error: missing SwiftPM XCFramework zip: ${zip_path}" >&2
    exit 1
  fi

  while IFS= read -r zip_entry; do
    [[ -n "${zip_entry}" ]] || continue
    entry_count=$((entry_count + 1))
    case "${zip_entry}" in
      "${framework}.xcframework"|\
      "${framework}.xcframework/"|\
      "${framework}.xcframework/"*) ;;
      *)
        echo "error: ${zip_path} must contain ${framework}.xcframework at the archive root; found ${zip_entry}" >&2
        exit 1
        ;;
    esac
  done < <(unzip -Z1 "${zip_path}")

  if [[ "${entry_count}" -eq 0 ]]; then
    echo "error: ${zip_path} is empty" >&2
    exit 1
  fi

  if ! unzip -Z1 "${zip_path}" | grep -qx "${framework}.xcframework/Info.plist"; then
    echo "error: ${zip_path} does not contain ${framework}.xcframework/Info.plist" >&2
    exit 1
  fi

  recorded_checksum="$(awk -v artifact="${artifact_path}" '$2 == artifact { print $1 }' "${swiftpm_checksum_file}")"
  if [[ -z "${recorded_checksum}" ]]; then
    echo "error: missing SwiftPM checksum entry for ${artifact_path}" >&2
    exit 1
  fi

  computed_checksum="$(swift package compute-checksum "${zip_path}")"
  if [[ "${computed_checksum}" != "${recorded_checksum}" ]]; then
    echo "error: SwiftPM checksum mismatch for ${zip_path}: ${computed_checksum}, expected ${recorded_checksum}" >&2
    exit 1
  fi

  if ! grep -Fq "${framework}.xcframework.zip" "${remote_package_dir}/Package.swift"; then
    echo "error: remote Swift package does not reference ${framework}.xcframework.zip" >&2
    exit 1
  fi

  if ! grep -Fq "${recorded_checksum}" "${remote_package_dir}/Package.swift"; then
    echo "error: remote Swift package does not reference checksum for ${framework}" >&2
    exit 1
  fi
}

for framework in "${required_frameworks[@]}"; do
  require_swiftpm_artifact "${framework}"
done

swiftpm_dump_tmp="$(mktemp -d "${TMPDIR:-/tmp}/brainflow-remote-package.XXXXXX")"
trap 'rm -rf "${swiftpm_dump_tmp}"' EXIT
(
  cd "${remote_package_dir}"
  export HOME="${swiftpm_dump_tmp}/home"
  export XDG_CACHE_HOME="${swiftpm_dump_tmp}/cache"
  export SWIFTPM_HOME="${swiftpm_dump_tmp}/swiftpm"
  mkdir -p "${HOME}" "${XDG_CACHE_HOME}" "${SWIFTPM_HOME}"
  swift package dump-package >/dev/null
)

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

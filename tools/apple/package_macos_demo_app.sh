#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
XCFRAMEWORKS_DIR="${BRAINFLOW_APPLE_XCFRAMEWORKS_DIR:-${ROOT_DIR}/build/apple_xcframeworks/XCFrameworks}"
OUTPUT_APP="${1:-${ROOT_DIR}/build/apple_xcframeworks/BrainFlowMacDemo.app}"
CONFIGURATION="${BRAINFLOW_MAC_DEMO_CONFIGURATION:-release}"

command -v swift >/dev/null || { echo "error: swift is required" >&2; exit 1; }
command -v codesign >/dev/null || { echo "error: codesign is required" >&2; exit 1; }

if [[ ! -d "${XCFRAMEWORKS_DIR}" ]]; then
  echo "error: missing XCFramework directory: ${XCFRAMEWORKS_DIR}" >&2
  exit 1
fi

swift_configuration_flag=()
swift_build_dir="debug"
if [[ "${CONFIGURATION}" == "release" ]]; then
  swift_configuration_flag=(-c release)
  swift_build_dir="release"
fi

(
  cd "${ROOT_DIR}/swift_package"
  swift build "${swift_configuration_flag[@]}" --product BrainFlowMacDemo
)

executable="${ROOT_DIR}/swift_package/.build/${swift_build_dir}/BrainFlowMacDemo"
if [[ ! -x "${executable}" ]]; then
  echo "error: missing built executable: ${executable}" >&2
  exit 1
fi

rm -rf "${OUTPUT_APP}"
mkdir -p "${OUTPUT_APP}/Contents/MacOS" "${OUTPUT_APP}/Contents/Frameworks" "${OUTPUT_APP}/Contents/Resources"
cp "${executable}" "${OUTPUT_APP}/Contents/MacOS/BrainFlowMacDemo"
cp "${ROOT_DIR}/swift_package/examples/apps/macos/BrainFlowMacDemo/PrivacyInfo.xcprivacy" "${OUTPUT_APP}/Contents/Resources/PrivacyInfo.xcprivacy"
cp "${ROOT_DIR}/swift_package/examples/apps/macos/BrainFlowMacDemo/BrainFlowMacDemo.entitlements" "${OUTPUT_APP}/Contents/Resources/BrainFlowMacDemo.entitlements"

cat > "${OUTPUT_APP}/Contents/Info.plist" <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleDevelopmentRegion</key>
  <string>en</string>
  <key>CFBundleExecutable</key>
  <string>BrainFlowMacDemo</string>
  <key>CFBundleIdentifier</key>
  <string>org.brainflow.demo.macos</string>
  <key>CFBundleInfoDictionaryVersion</key>
  <string>6.0</string>
  <key>CFBundleName</key>
  <string>BrainFlowMacDemo</string>
  <key>CFBundlePackageType</key>
  <string>APPL</string>
  <key>CFBundleShortVersionString</key>
  <string>1.0</string>
  <key>CFBundleVersion</key>
  <string>1</string>
  <key>LSMinimumSystemVersion</key>
  <string>12.0</string>
  <key>NSPrincipalClass</key>
  <string>NSApplication</string>
</dict>
</plist>
PLIST

select_macos_framework_slice() {
  local framework_name="$1"
  local xcframework="${XCFRAMEWORKS_DIR}/${framework_name}.xcframework"
  if [[ ! -d "${xcframework}" ]]; then
    echo "error: missing BrainFlow XCFramework ${xcframework}" >&2
    exit 1
  fi
  find "${xcframework}" -path "*/${framework_name}.framework" -type d | grep '/macos-' | head -n 1 || true
}

for framework_name in BoardController DataHandler MLModule; do
  src_framework="$(select_macos_framework_slice "${framework_name}")"
  if [[ -z "${src_framework}" ]]; then
    echo "error: unable to select macOS slice for ${framework_name}" >&2
    exit 1
  fi
  cp -R "${src_framework}" "${OUTPUT_APP}/Contents/Frameworks/${framework_name}.framework"
  codesign --force --sign - --timestamp=none "${OUTPUT_APP}/Contents/Frameworks/${framework_name}.framework"
done

codesign --force --sign - --timestamp=none "${OUTPUT_APP}"

echo "BrainFlowMacDemo app bundle written to ${OUTPUT_APP}"

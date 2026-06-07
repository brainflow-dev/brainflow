#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
ARTIFACT_DIR="${1:-${ROOT_DIR}/build/apple_xcframeworks}"
ARTIFACT_DIR="$(cd "${ARTIFACT_DIR}" && pwd)"
PACKAGE_DIR="${ARTIFACT_DIR}/BrainFlowSwiftBinaryPackage"

command -v swift >/dev/null || { echo "error: swift is required" >&2; exit 1; }

if [[ ! -f "${PACKAGE_DIR}/Package.swift" ]]; then
  echo "error: missing generated Swift binary package: ${PACKAGE_DIR}" >&2
  exit 1
fi

smoke_dir="$(mktemp -d "${TMPDIR:-/tmp}/brainflow-binary-smoke.XXXXXX")"
trap 'rm -rf "${smoke_dir}"' EXIT

mkdir -p "${smoke_dir}/Sources/BrainFlowBinarySmoke"
mkdir -p "${smoke_dir}/.cache" "${smoke_dir}/.swiftpm" "${smoke_dir}/ModuleCache" "${smoke_dir}/home"
export CLANG_MODULE_CACHE_PATH="${CLANG_MODULE_CACHE_PATH:-${smoke_dir}/ModuleCache}"
export HOME="${BRAINFLOW_SWIFT_BINARY_SMOKE_HOME:-${smoke_dir}/home}"
export XDG_CACHE_HOME="${XDG_CACHE_HOME:-${smoke_dir}/.cache}"
export SWIFTPM_HOME="${SWIFTPM_HOME:-${smoke_dir}/.swiftpm}"

cat > "${smoke_dir}/Package.swift" <<PACKAGE
// swift-tools-version: 5.9

import PackageDescription

let package = Package(
    name: "BrainFlowBinarySmoke",
    platforms: [
        .macOS(.v12)
    ],
    dependencies: [
        .package(path: "${PACKAGE_DIR}")
    ],
    targets: [
        .executableTarget(
            name: "BrainFlowBinarySmoke",
            dependencies: [
                .product(name: "BrainFlow", package: "BrainFlowSwiftBinaryPackage")
            ]
        )
    ]
)
PACKAGE

cat > "${smoke_dir}/Sources/BrainFlowBinarySmoke/main.swift" <<'SWIFT'
import BrainFlow
import Foundation

let board = try BoardShim(board_id: .SYNTHETIC_BOARD)
try board.prepare_session()
try board.start_stream(buffer_size: 45000)
Thread.sleep(forTimeInterval: 1.0)
try board.stop_stream()
let data = try board.get_board_data()
try board.release_session()

let expectedRows = try BoardShim.get_num_rows(board_id: BoardIds.SYNTHETIC_BOARD.rawValue)
let samples = data.first?.count ?? 0

guard data.count == expectedRows, samples > 0 else {
    fputs("Binary package smoke failed: rows=\(data.count) expected=\(expectedRows) samples=\(samples)\n", stderr)
    exit(1)
}

print("BrainFlow Swift binary package smoke passed: rows=\(data.count) samples=\(samples)")
SWIFT

(
  cd "${smoke_dir}"
  swift run \
    --disable-sandbox \
    --disable-dependency-cache \
    --manifest-cache local \
    --cache-path "${smoke_dir}/.cache/swiftpm" \
    --config-path "${smoke_dir}/.swiftpm/config" \
    --security-path "${smoke_dir}/.swiftpm/security" \
    --scratch-path "${smoke_dir}/.build" \
    BrainFlowBinarySmoke
)

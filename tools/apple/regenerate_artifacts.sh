#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
ARTIFACT_DIR="${BRAINFLOW_APPLE_ARTIFACT_DIR:-${ROOT_DIR}/build/apple_xcframeworks}"

"${ROOT_DIR}/tools/apple/build_xcframeworks.sh" \
  --output "${ARTIFACT_DIR}" \
  "$@"

"${ROOT_DIR}/tools/apple/verify_xcframeworks.sh" "${ARTIFACT_DIR}"

echo "Regenerated Apple artifacts in ${ARTIFACT_DIR}"

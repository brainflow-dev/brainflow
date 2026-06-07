# Agent Notes

## Apple XCFramework Artifacts

Apple binary artifacts are generated build outputs. They contain framework-wrapped XCFrameworks
for iOS device, iOS simulator, and macOS app integration, plus a generated
`BrainFlowSwiftBinaryPackage` for app developers.

Regenerate the artifacts from the repository root:

```bash
tools/apple/regenerate_artifacts.sh
```

Verify an existing artifact tree:

```bash
tools/apple/verify_xcframeworks.sh build/apple_xcframeworks
```

The regeneration script builds native BrainFlow Apple slices, packages the XCFrameworks, verifies
the required core frameworks, and creates `BrainFlowAppleXCFrameworks.zip` plus checksum files.

By default, `tools/apple/regenerate_artifacts.sh` and `tools/apple/build_xcframeworks.sh` write to
`build/apple_xcframeworks`. Do not commit `build/`, `build_apple/`, `swift_package/Artifacts/Apple`,
or local `installed/` outputs.

The iOS demo and macOS packaging script default to `build/apple_xcframeworks/XCFrameworks`.
CI may override artifact paths with
`BRAINFLOW_APPLE_XCFRAMEWORKS_DIR`.

When changing Apple artifact generation, regenerate locally, run verification, and let CI upload
`BrainFlowAppleXCFrameworks.zip` plus `BrainFlowAppleXCFrameworks.zip.sha256`, `checksums.sha256`,
and `manifest.json` as the distributable artifact set. Generated framework headers and binaries
should come from the scripts, not from files copied into the repository.

For a BrainFlow release or Apple-library refresh, build from the release commit/tag with
`BRAINFLOW_VERSION` set, regenerate artifacts, verify the generated tree, smoke-test the generated
Swift binary package, and validate the iOS/macOS sample apps against the regenerated XCFrameworks.
Do not manually patch release frameworks after generation; update source and rerun the script.

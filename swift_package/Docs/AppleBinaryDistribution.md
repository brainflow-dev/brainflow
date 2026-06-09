# Apple Binary Distribution

BrainFlow Swift source bindings can be built directly from this repository, but production iOS
and macOS apps should consume signed, embedded Apple frameworks instead of loose local dylibs.
The Apple packaging workflow creates framework-wrapped XCFrameworks and a generated SwiftPM
binary package for app developers.

## Generated Artifacts

Regenerate the Apple artifacts from source:

```bash
tools/apple/regenerate_artifacts.sh
```

Verify the generated artifact tree:

```bash
tools/apple/verify_xcframeworks.sh build/apple_xcframeworks
```

The default output is:

- `build/apple_xcframeworks/XCFrameworks/*.xcframework`
- `build/apple_xcframeworks/BrainFlowSwiftBinaryPackage`
- `build/apple_xcframeworks/BrainFlowSwiftPackageRemote`
- `build/apple_xcframeworks/SwiftPMArtifacts/*.xcframework.zip`
- `build/apple_xcframeworks/BrainFlowAppleXCFrameworks.zip`

The generated package contains the BrainFlow Swift API plus binary targets for:

- `BoardController.xcframework`
- `DataHandler.xcframework`
- `MLModule.xcframework`

The artifact directory also includes `checksums.sha256` and
`BrainFlowAppleXCFrameworks.zip.sha256`; `tools/apple/verify_xcframeworks.sh` validates both.
It also includes `swiftpm-checksums.txt` and `swiftpm-checksums.json`, generated with
`swift package compute-checksum` for the SwiftPM release ZIPs.

Generated artifacts are not committed to the source repository. CI uploads
`BrainFlowAppleXCFrameworks.zip` as a workflow artifact for complete archive downloads. Releases
should also publish the individual files in `SwiftPMArtifacts` because SwiftPM URL-based binary
targets expect a ZIP archive with the `.xcframework` at the archive root.

Add `BrainFlowSwiftBinaryPackage` to an app in Xcode or with Swift Package Manager. Xcode embeds
and signs the binary frameworks during app builds.

Use `BrainFlowSwiftPackageRemote` as the release-facing Swift package template when publishing
from GitHub Releases, a CDN, or another public HTTPS host. It declares URL-based binary targets for
`BoardController`, `DataHandler`, and `MLModule` using the checksums from
`swiftpm-checksums.json`.

The artifact directory may also include optional board vendor XCFrameworks such as Muse, Ganglion,
BrainBit, or NeuroSDK libraries. Those are not dependencies of the core `BrainFlow` Swift product;
embed the optional vendor frameworks explicitly when the app enables boards that require them.

## Supported Slices

The packaging script builds and verifies:

- macOS universal framework slices
- iOS device framework slices
- iOS simulator framework slices

Core BrainFlow libraries are required for every slice. Optional vendor libraries are packaged only
for platforms where the selected native build options produce valid Apple binaries.

## Optional Native Features

The default artifact build keeps optional native SDKs disabled for a small, App Store-friendly
synthetic-board package. Enable optional features with environment variables:

```bash
BRAINFLOW_APPLE_BUILD_BLE=ON \
BRAINFLOW_APPLE_BUILD_BLUETOOTH=ON \
BRAINFLOW_APPLE_BUILD_ONNX=ON \
tools/apple/build_xcframeworks.sh
```

Only ship optional vendor frameworks that are supported on the Apple platform you target and that
match your App Store privacy and permission answers.

## Release Maintenance

Apple library releases should be reproducible from the same source revision as the BrainFlow
release. For each BrainFlow release or Apple-library refresh:

1. Build from a clean checkout of the release commit or tag.
2. Set `BRAINFLOW_VERSION` to the release version. If the binary assets will not be hosted under
   `https://github.com/brainflow-dev/brainflow/releases/download/$BRAINFLOW_VERSION`, set
   `BRAINFLOW_APPLE_RELEASE_BASE_URL` to the exact public URL prefix for the `.xcframework.zip`
   assets.
3. Run `tools/apple/regenerate_artifacts.sh`.
4. Verify `build/apple_xcframeworks` with `tools/apple/verify_xcframeworks.sh`.
5. Run the generated Swift binary package smoke test:

```bash
tools/apple/test_swift_binary_package.sh build/apple_xcframeworks
```

6. Build the iOS demo and package the macOS demo against `build/apple_xcframeworks/XCFrameworks`.
7. Publish `SwiftPMArtifacts/BoardController.xcframework.zip`,
   `SwiftPMArtifacts/DataHandler.xcframework.zip`, `SwiftPMArtifacts/MLModule.xcframework.zip`,
   `swiftpm-checksums.txt`, `swiftpm-checksums.json`, `BrainFlowAppleXCFrameworks.zip`,
   `BrainFlowAppleXCFrameworks.zip.sha256`, `checksums.sha256`, and `manifest.json` from the same
   CI run or GitHub Release.

The generated `manifest.json` records the BrainFlow version, source revision, Xcode, CMake, Ninja,
deployment targets, optional native feature flags, SwiftPM asset URL base, and SwiftPM binary
target checksums. Use it as the compatibility record for supporting downstream developers and for
reproducing a release later.

When BrainFlow native headers or libraries change, do not edit framework contents by hand. Update
the source, rerun the Apple artifact script, and release the newly generated zip plus checksums.
The packaging script copies public headers from the current source/install tree into framework
slices as part of generation.

For a remote Swift Package distribution, publish each XCFramework archive from a release URL and
use URL-based `binaryTarget` declarations with checksums generated by
`swift package compute-checksum`. The archive must contain the `.xcframework` at the ZIP root,
which `tools/apple/build_xcframeworks.sh` enforces for files in `SwiftPMArtifacts`. For local
development or downloaded release archives, the generated `BrainFlowSwiftBinaryPackage` uses
path-based binary targets. Apple documents both distribution modes in
https://developer.apple.com/documentation/xcode/distributing-binary-frameworks-as-swift-packages.

Apple's XCFramework guidance is the baseline for this workflow:
https://developer.apple.com/documentation/xcode/creating-a-multi-platform-binary-framework-bundle.

## App Store Practice

Production app builds should follow normal Apple SDK distribution rules:

- Use XCFrameworks for multi-platform binary distribution.
- Embed dynamic frameworks in the app bundle under `Frameworks`.
- Let Xcode sign embedded frameworks with the app during build/archive.
- Do not ship simulator slices in iOS device archives.
- Verify `@rpath/<Framework>.framework/<Framework>` install names.
- Keep privacy manifests and app privacy answers aligned with the actual native binaries shipped.
- For public SDK-style distribution, sign release XCFrameworks with an Apple Developer Program
  identity when the release process has access to one. CI smoke builds may use ad-hoc signing only
  for local validation.

## Sample App Checks

The iOS demo embeds framework slices from `build/apple_xcframeworks/XCFrameworks` by default.
Override the artifact directory with `BRAINFLOW_APPLE_XCFRAMEWORKS_DIR`.

The macOS SwiftUI demo can be packaged as an app bundle:

```bash
tools/apple/package_macos_demo_app.sh
```

Run app smoke tests with the synthetic board and without `BRAINFLOW_LIB_DIR` to verify that runtime
loading works from embedded frameworks, not from local development directories.

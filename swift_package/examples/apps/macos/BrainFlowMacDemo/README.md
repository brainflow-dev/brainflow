# BrainFlow macOS Demo

The buildable macOS SwiftUI demo target lives in `swift_package` as `BrainFlowMacDemo`.

For Mac App Store distribution, use this folder's entitlements and privacy manifest as starting assets in an Xcode app target. Embed BrainFlow XCFramework products in the app bundle, sign them with the same team, and keep the sandbox entitlement enabled.

Release placeholders to replace before upload:

- Bundle ID: `org.brainflow.demo.macos`
- Signing team and provisioning profile
- App icon
- Mac App Store screenshots
- Final privacy answers for any real-board connectivity features

Source-development smoke test:

```bash
cd swift_package
BRAINFLOW_LIB_DIR=../installed/lib swift run BrainFlowMacDemo
```

App-bundle smoke test:

```bash
tools/apple/regenerate_artifacts.sh
tools/apple/package_macos_demo_app.sh
```

The app bundle is written to `build/apple_xcframeworks/BrainFlowMacDemo.app` and embeds
`BoardController.framework`, `DataHandler.framework`, and `MLModule.framework` from
`build/apple_xcframeworks/XCFrameworks` by default. Run the app without `BRAINFLOW_LIB_DIR` to
verify production-style framework loading from the app bundle.

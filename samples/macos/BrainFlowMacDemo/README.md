# BrainFlow macOS Demo

The buildable macOS SwiftUI demo target lives in `swift_package` as `BrainFlowMacDemo`.

For Mac App Store distribution, use this folder's `Info.plist`, entitlements, and privacy manifest as starting assets in an Xcode app target. Embed the BrainFlow dynamic libraries or XCFrameworks in the app bundle, sign them with the same team, and keep the sandbox entitlement enabled.

Release placeholders to replace before upload:

- Bundle ID: `org.brainflow.demo.macos`
- Signing team and provisioning profile
- App icon
- Mac App Store screenshots
- Final privacy answers for any real-board connectivity features

Local smoke test:

```bash
cd swift_package
BRAINFLOW_LIB_DIR=../installed/lib swift run BrainFlowMacDemo
```

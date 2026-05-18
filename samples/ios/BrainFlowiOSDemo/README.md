# BrainFlow iOS Demo

This sample is a minimal SwiftUI app that exercises the BrainFlow Swift package with the synthetic board, so it does not need external hardware for App Review or TestFlight smoke testing.

To make a distributable app, create an iOS app target in Xcode, add `swift_package` as a local package dependency, add these source files, and embed signed BrainFlow native libraries or XCFrameworks that include the iOS slices you intend to ship.

App Store placeholders to replace before upload:

- Bundle ID: `org.brainflow.demo.ios`
- Display name and app icon
- Signing team and provisioning profile
- Screenshots for required iPhone/iPad sizes
- App privacy answers matching the final native libraries and any real-board permissions

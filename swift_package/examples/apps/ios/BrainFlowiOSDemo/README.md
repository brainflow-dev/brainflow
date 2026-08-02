# BrainFlow iOS Demo

This sample is a normal Xcode iOS application that exercises the BrainFlow Swift package with the synthetic board, so it does not need external hardware for simulator, TestFlight, or App Review smoke testing.

## Run In Simulator

From the repository root, regenerate the Apple artifacts first:

```bash
tools/apple/regenerate_artifacts.sh
tools/apple/verify_xcframeworks.sh build/apple_xcframeworks
```

Then open `BrainFlowiOSDemo.xcodeproj` in Xcode, select an iPhone simulator, and run the `BrainFlowiOSDemo` scheme. The app target links the local Swift package at `../../../..` and embeds native framework slices from `../../../../../build/apple_xcframeworks/XCFrameworks` by default.

Set `BRAINFLOW_APPLE_XCFRAMEWORKS_DIR` in the Xcode build environment to use a different artifact directory.

For command-line smoke testing, pass `--autorun` as a launch argument. The app starts the synthetic board, records data, stops streaming, releases the session, and displays the row/sample count and EEG plot.

## App Store Preparation

The simulator build is not enough for App Store distribution. For an iPhone archive, use the generated XCFrameworks and ensure the archive embeds signed `iphoneos` framework slices for:

- `BoardController.framework`
- `DataHandler.framework`
- `MLModule.framework`

Muse native BLE boards require BrainFlow native libraries built with BLE support for the target platform. The demo exposes board selection plus serial number, MAC address, and timeout fields for native BLE connections.

Before upload, also replace the placeholders below.

App Store placeholders to replace before upload:

- Bundle ID: `org.brainflow.demo.ios`
- Display name and app icon
- Signing team and provisioning profile
- Screenshots for required iPhone/iPad sizes
- App privacy answers matching the final native libraries and any real-board permissions

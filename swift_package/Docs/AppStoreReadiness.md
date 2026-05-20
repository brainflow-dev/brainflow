# App Store Readiness

This checklist is intentionally separate from the sample source because final App Store submission requires a developer account, bundle IDs, certificates, provisioning profiles, App Store Connect records, screenshots, and final product metadata.

## Shared

- Build with the current App Store-required SDK in Xcode.
- Replace placeholder bundle IDs.
- Add production app icons and screenshots.
- Keep the synthetic-board demo path available so App Review can exercise the app without external hardware.
- Embed BrainFlow native libraries or XCFrameworks in the app bundle and sign them with the app.
- Confirm final privacy answers reflect real-board connectivity, Bluetooth, networking, files, and any third-party native dependencies actually shipped.
- Run an archive build and install it on a physical device or clean Mac before upload.

## iOS

- Use `examples/apps/ios/BrainFlowiOSDemo` as the Xcode app project.
- Keep `swift_package` as the local package dependency.
- Provide iOS-compatible BrainFlow native binaries. The current high-level Swift package compiles for iOS, but the app can only run BrainFlow calls when matching native libraries are embedded.
- For Muse native BLE boards, build BrainFlow native libraries with BLE support enabled for the target platform and keep the Bluetooth privacy string in the app plist.
- Keep permissions minimal. The synthetic-board demo needs no network or file permissions.
- Test via TestFlight before App Store submission.

## macOS

- Use `swift_package` product `BrainFlowMacDemo` for local development, or create an Xcode app target for App Store archiving.
- Add the files from `examples/apps/macos/BrainFlowMacDemo`.
- Enable App Sandbox.
- Embed and sign BrainFlow dylibs/XCFrameworks.
- Verify dynamic loading works inside the archived app bundle, not only with `BRAINFLOW_LIB_DIR`.

## Production Gate

- Swift package builds.
- Swift tests pass with native libraries present.
- CLI smoke test succeeds with the synthetic board.
- iOS and macOS app targets launch, handle missing native libraries gracefully, and run the synthetic-board workflow when libraries are embedded.
- Accessibility labels and dynamic text behavior are reviewed in the sample apps.
- Crash logs are clean after repeated start, stop, read, and release cycles.

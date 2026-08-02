.. _app-store-readiness-label:

App Store Readiness
===================

This checklist is intentionally separate from the sample source because final App Store submission
requires a developer account, bundle IDs, certificates, provisioning profiles, App Store Connect
records, screenshots, and final product metadata.

Shared
------

- Build with the current App Store-required SDK in Xcode.
- Replace placeholder bundle IDs.
- Add production app icons and screenshots.
- Keep the synthetic-board demo path available so App Review can exercise the app without external
  hardware.
- Embed BrainFlow XCFramework products in the app bundle and sign them with the app.
- Use the generated :code:`BrainFlowSwiftBinaryPackage` for production app integration. Do not rely
  on :code:`BRAINFLOW_LIB_DIR`, local :code:`installed/lib` folders, or loose development dylibs in
  App Store builds.
- Confirm final privacy answers reflect real-board connectivity, Bluetooth, networking, files, and
  any third-party native dependencies actually shipped.
- Run an archive build and install it on a physical device or clean Mac before upload.
- Verify the archive contains only device slices for iOS, with embedded framework install names in
  the form :code:`@rpath/<Framework>.framework/<Framework>`.

iOS
---

- Use :code:`examples/apps/ios/BrainFlowiOSDemo` as the Xcode app project.
- Use the generated Swift binary package or embed framework slices from
  :code:`build/apple_xcframeworks/XCFrameworks`.
- Provide iOS-compatible BrainFlow native binaries through XCFrameworks. The high-level Swift
  package compiles for iOS, but BrainFlow calls can only run when matching native frameworks are
  embedded and signed.
- For Muse native BLE boards, build BrainFlow native libraries with BLE support enabled for the
  target platform and keep the Bluetooth privacy string in the app plist.
- Keep permissions minimal. The synthetic-board demo needs no network or file permissions.
- Test via TestFlight before App Store submission.

macOS
-----

- Use :code:`swift_package` product :code:`BrainFlowMacDemo` for local development, or package it
  with :code:`tools/apple/package_macos_demo_app.sh` for app-bundle smoke testing.
- Add the files from :code:`examples/apps/macos/BrainFlowMacDemo`.
- Enable App Sandbox.
- Embed and sign BrainFlow XCFramework products.
- Verify dynamic loading works inside the app bundle, not only with :code:`BRAINFLOW_LIB_DIR`.

Production Gate
---------------

- Swift package builds.
- Swift tests pass with native libraries present.
- CLI smoke test succeeds with the synthetic board.
- :code:`tools/apple/build_xcframeworks.sh` and :code:`tools/apple/verify_xcframeworks.sh` pass.
- :code:`tools/apple/regenerate_artifacts.sh` refreshes :code:`build/apple_xcframeworks`, and
  :code:`tools/apple/verify_xcframeworks.sh build/apple_xcframeworks` passes.
- The Apple release artifact set includes the individual SwiftPM XCFramework zips,
  :code:`swiftpm-checksums.txt`, :code:`swiftpm-checksums.json`,
  :code:`BrainFlowSwiftPackageRemote`, :code:`BrainFlowAppleXCFrameworks.zip`,
  :code:`BrainFlowAppleXCFrameworks.zip.sha256`, :code:`checksums.sha256`, and
  :code:`manifest.json` from the same build.
- :code:`manifest.json` records the BrainFlow version, source revision, toolchain versions,
  deployment targets, optional native feature flags, SwiftPM release URL base, and binary target
  checksums.
- A clean app consumes :code:`BrainFlowSwiftBinaryPackage` without building native BrainFlow
  locally.
- iOS and macOS app targets launch, handle missing native frameworks gracefully, and run the
  synthetic-board workflow when frameworks are embedded.
- Accessibility labels and dynamic text behavior are reviewed in the sample apps.
- Crash logs are clean after repeated start, stop, read, and release cycles.

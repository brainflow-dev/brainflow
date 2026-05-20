# BrainFlow iOS Demo

This sample is a normal Xcode iOS application that exercises the BrainFlow Swift package with the synthetic board, so it does not need external hardware for simulator, TestFlight, or App Review smoke testing.

## Run In Simulator

From the repository root, build simulator native libraries first:

```bash
cmake -S . -B build_ios_sim -G Ninja \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_SYSROOT=iphonesimulator \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_INSTALL_PREFIX=installed_ios_sim \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_BLUETOOTH=OFF \
  -DBUILD_BLE=OFF \
  -DBUILD_ONNX=OFF \
  -DBUILD_TESTS=OFF \
  -DBUILD_SYNCHRONI_SDK=OFF
ninja -C build_ios_sim install
```

Then open `BrainFlowiOSDemo.xcodeproj` in Xcode, select an iPhone simulator, and run the `BrainFlowiOSDemo` scheme. The app target links the local Swift package at `../../../..` and embeds native libraries from `../../../../../installed_ios_sim/lib` by default.

For command-line smoke testing, pass `--autorun` as a launch argument. The app starts the synthetic board, records data, stops streaming, releases the session, and displays the row/sample count and EEG plot.

## App Store Preparation

The simulator build is not enough for App Store distribution. For an iPhone archive, build signed `iphoneos` native libraries into `installed_ios/lib` or set `BRAINFLOW_IOS_NATIVE_LIB_DIR` to a directory containing the device slices for:

- `libBoardController.dylib`
- `libDataHandler.dylib`
- `libMLModule.dylib`

Muse native BLE boards require BrainFlow native libraries built with BLE support for the target platform. The demo exposes board selection plus serial number, MAC address, and timeout fields for native BLE connections.

Before upload, also replace the placeholders below.

App Store placeholders to replace before upload:

- Bundle ID: `org.brainflow.demo.ios`
- Display name and app icon
- Signing team and provisioning profile
- Screenshots for required iPhone/iPad sizes
- App privacy answers matching the final native libraries and any real-board permissions

# BrainFlow Swift

Swift bindings call BrainFlow's native C ABI through runtime dynamic loading. Build the native libraries first, then point Swift at the installed library directory.

```bash
cd ..
python3 tools/build.py

cd swift_package
BRAINFLOW_LIB_DIR=../installed/lib swift test
BRAINFLOW_LIB_DIR=../installed/lib swift run brainflow-swift-cli
BRAINFLOW_LIB_DIR=../installed/lib swift run swift-brainflow-get-data
```

The Swift package does not vendor native BrainFlow binaries. Build native libraries from this repository and provide them at runtime. The loader searches `BRAINFLOW_LIB_DIR`, `DYLD_LIBRARY_PATH`, `LD_LIBRARY_PATH`, `installed/lib`, app bundle resources, and the current directory for:

- `libBoardController.dylib`
- `libDataHandler.dylib`
- `libMLModule.dylib`

On Linux the equivalent `.so` names are used.

## API Coverage

The package exposes Swift equivalents for the public Python/Java binding surface:

- `BoardShim`: session lifecycle, streamers, data reads, markers, config, board metadata, logging, versions.
- `DataFilter`: filters, denoising, FFT/IFFT, PSD, band powers, CSP, ICA, file IO, statistics, logging, versions.
- `MLModel`: model params, prepare/release, predict, logging, versions.
- `BrainFlowInputParams`, `BrainFlowModelParams`, `BrainFlowError`, and public constants/enums.

Swift in-place signal-processing methods take `inout [Double]`, for example:

```swift
var data = Array(0..<256).map { sin(Double($0) / 10.0) }
try DataFilter.perform_lowpass(
    data: &data,
    sampling_rate: 250,
    cutoff: 30.0,
    order: 4,
    filter_type: .BUTTERWORTH,
    ripple: 0.0
)
```

## Apps

- `swift run BrainFlowMacDemo` builds a simple macOS SwiftUI demo against the synthetic board.
- `examples/apps/ios/BrainFlowiOSDemo` contains an Xcode iOS app project with synthetic-board autorun, Muse/native BLE board selection, and an EEG plot.
- `examples/apps/macos/BrainFlowMacDemo` contains Mac App Store release-prep metadata for an Xcode app bundle.

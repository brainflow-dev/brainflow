// swift-tools-version: 5.9

import PackageDescription

let exampleTargets: [(product: String, target: String, path: String)] = [
    ("swift-brainflow-get-data", "SwiftBrainFlowGetDataExample", "examples/tests/brainflow_get_data"),
    ("swift-markers", "SwiftMarkersExample", "examples/tests/markers"),
    ("swift-read-write-file", "SwiftReadWriteFileExample", "examples/tests/read_write_file"),
    ("swift-downsampling", "SwiftDownsamplingExample", "examples/tests/downsampling"),
    ("swift-transforms", "SwiftTransformsExample", "examples/tests/transforms"),
    ("swift-signal-filtering", "SwiftSignalFilteringExample", "examples/tests/signal_filtering"),
    ("swift-denoising", "SwiftDenoisingExample", "examples/tests/denoising"),
    ("swift-band-power", "SwiftBandPowerExample", "examples/tests/band_power"),
    ("swift-eeg-metrics", "SwiftEEGMetricsExample", "examples/tests/eeg_metrics"),
    ("swift-ica", "SwiftICAExample", "examples/tests/ica")
]

let package = Package(
    name: "BrainFlow",
    platforms: [
        .macOS(.v12),
        .iOS(.v15)
    ],
    products: [
        .library(name: "BrainFlow", targets: ["BrainFlow"]),
        .executable(name: "brainflow-swift-cli", targets: ["BrainFlowCLI"]),
        .executable(name: "BrainFlowMacDemo", targets: ["BrainFlowMacDemo"])
    ] + exampleTargets.map { .executable(name: $0.product, targets: [$0.target]) },
    targets: [
        .target(
            name: "BrainFlow"
        ),
        .target(
            name: "BrainFlowExampleSupport",
            dependencies: ["BrainFlow"],
            path: "examples/tests/support"
        ),
        .executableTarget(
            name: "BrainFlowCLI",
            dependencies: ["BrainFlow"]
        ),
        .executableTarget(
            name: "BrainFlowMacDemo",
            dependencies: ["BrainFlow"]
        ),
        .testTarget(
            name: "BrainFlowTests",
            dependencies: ["BrainFlow"]
        )
    ] + exampleTargets.map {
        .executableTarget(
            name: $0.target,
            dependencies: ["BrainFlow", "BrainFlowExampleSupport"],
            path: $0.path
        )
    }
)

// swift-tools-version: 5.9

import PackageDescription

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
    ],
    targets: [
        .target(
            name: "BrainFlow"
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
    ]
)

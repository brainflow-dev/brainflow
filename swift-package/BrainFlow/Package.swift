// swift-tools-version:5.3
import PackageDescription

let package = Package(
    name: "BrainFlow",
    platforms: [
        .macOS(.v10_15), .iOS(.v13)
    ],
    products: [
        // Products define the executables and libraries a package produces, and make them visible to other packages.
        .library(name: "BrainFlow",
                 targets: ["BrainFlow", "BoardController", "DataHandler", "MLModule", "BrainBitLib"])
    ],
    dependencies: [
        .package(name: "swift-numerics",
                 url: "https://github.com/apple/swift-numerics.git", .upToNextMajor(from: "1.0.0"))
    ],
    targets: [
        .target(
            name: "BrainFlow",
            dependencies: [.product(name: "Numerics", package: "swift-numerics"),
                           .target(name: "BoardController")]
        ),
        .binaryTarget(
            name: "BoardController",
            path: "BoardController.xcframework"
        ),
        .binaryTarget(
            name: "DataHandler",
            path: "DataHandler.xcframework"
        ),
        .binaryTarget(
            name: "MLModule",
            path: "MLModule.xcframework"
        ),
        .binaryTarget(
            name: "BrainBitLib",
            path: "BrainBitLib.xcframework"
        ),
        .testTarget(
            name: "BrainFlowTests",
            dependencies: ["BrainFlow", .product(name: "Numerics", package: "swift-numerics")],
            sources: ["BoardShimTests.swift",
                      "BrainFlowCItests.swift",
                      "BrainFlowTests.swift",
                      "DataFilterTests.swift"]
        )
    ]
)


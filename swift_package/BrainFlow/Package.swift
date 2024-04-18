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
                 targets: ["BrainFlow"])
    ],
    dependencies: [
        .package(name: "swift-numerics",
                 url: "https://github.com/apple/swift-numerics.git", .upToNextMajor(from: "1.0.0"))
    ],
    targets: [
        .target(
            name: "BrainFlow",
            dependencies: [.product(name: "Numerics", package: "swift-numerics")]
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


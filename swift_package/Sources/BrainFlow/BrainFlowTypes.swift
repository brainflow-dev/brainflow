import Foundation

public struct Complex: Equatable, Sendable {
    public var real: Double
    public var imag: Double

    public init(real: Double, imag: Double) {
        self.real = real
        self.imag = imag
    }
}

public struct WaveletTransform: Equatable, Sendable {
    public var coefficients: [Double]
    public var decomposition_lengths: [Int]

    public init(coefficients: [Double], decomposition_lengths: [Int]) {
        self.coefficients = coefficients
        self.decomposition_lengths = decomposition_lengths
    }
}

public struct PSD: Equatable, Sendable {
    public var ampl: [Double]
    public var freq: [Double]

    public init(ampl: [Double], freq: [Double]) {
        self.ampl = ampl
        self.freq = freq
    }
}

public struct BandPowerResult: Equatable, Sendable {
    public var average: [Double]
    public var stddev: [Double]

    public init(average: [Double], stddev: [Double]) {
        self.average = average
        self.stddev = stddev
    }
}

public struct SignalQualityFlags: OptionSet, Sendable {
    public let rawValue: CInt

    public init(rawValue: CInt) {
        self.rawValue = rawValue
    }

    public static let nonfinite = Self(rawValue: 1)
    public static let clipped = Self(rawValue: 2)
    public static let flatline = Self(rawValue: 4)
}

/// Thresholds for opt-in, whole-channel quality checks.
public struct SignalQualityOptions: Equatable, Sendable {
    public var enable_clipping: Bool
    public var lower_bound: Double
    public var upper_bound: Double
    public var max_clipped_percentage: Double
    public var flatline_tolerance: Double
    public var max_flatline_seconds: Double
    public var min_channels: Int

    public init(enable_clipping: Bool = false, lower_bound: Double = 0, upper_bound: Double = 0,
                max_clipped_percentage: Double = 0, flatline_tolerance: Double = 0,
                max_flatline_seconds: Double = 0.5, min_channels: Int = 1) {
        self.enable_clipping = enable_clipping
        self.lower_bound = lower_bound
        self.upper_bound = upper_bound
        self.max_clipped_percentage = max_clipped_percentage
        self.flatline_tolerance = flatline_tolerance
        self.max_flatline_seconds = max_flatline_seconds
        self.min_channels = min_channels
    }
}

public struct ChannelQuality: Equatable, Sendable {
    public let channel: Int
    public let flags: SignalQualityFlags
    public let nonfinite_percentage: Double
    public let clipped_percentage: Double
    public let flatline_percentage: Double
    public let longest_flatline_seconds: Double
}

public struct SignalQualityResult: Equatable, Sendable {
    public let channels: [Int]
    public let quality: [ChannelQuality]
    public let used_channels: [Int]
    public let rejected_channels: [Int]
    public let quality_policy: SignalQualityOptions
}

public enum BandPowerQualityReason: String, Sendable {
    case ok
    case insufficientChannels = "insufficient_channels"
    case noBandPower = "no_band_power"
}

public struct BandPowerQualityResult: Equatable, Sendable {
    public let signal_quality: SignalQualityResult
    public let valid: Bool
    public let reason: BandPowerQualityReason
    /// Normalized means, or nil when invalid.
    public let avg_band_powers: [Double]?
    /// Coefficients of variation, or nil when invalid.
    public let stddev_band_powers: [Double]?
}

public struct CSPResult: Equatable, Sendable {
    public var filters: [[Double]]
    public var eigenvalues: [Double]

    public init(filters: [[Double]], eigenvalues: [Double]) {
        self.filters = filters
        self.eigenvalues = eigenvalues
    }
}

public struct ICAResult: Equatable, Sendable {
    public var w: [[Double]]
    public var k: [[Double]]
    public var a: [[Double]]
    public var s: [[Double]]

    public init(w: [[Double]], k: [[Double]], a: [[Double]], s: [[Double]]) {
        self.w = w
        self.k = k
        self.a = a
        self.s = s
    }
}

public struct FrequencyBand: Equatable, Sendable {
    public var start: Double
    public var stop: Double

    public init(start: Double, stop: Double) {
        self.start = start
        self.stop = stop
    }
}

enum BrainFlowArray {
    static func reshape_data_to_1d(num_rows: Int, num_cols: Int, buf: [[Double]]) -> [Double] {
        var output = [Double](repeating: 0.0, count: num_rows * num_cols)
        for col in 0..<num_cols {
            for row in 0..<num_rows {
                output[row * num_cols + col] = buf[row][col]
            }
        }
        return output
    }

    static func reshape_data_to_2d(num_rows: Int, num_cols: Int, linear_buffer: [Double]) -> [[Double]] {
        guard num_rows > 0, num_cols > 0 else { return [] }
        return (0..<num_rows).map { row in
            let start = row * num_cols
            return Array(linear_buffer[start..<(start + num_cols)])
        }
    }

    static func validateRectangular(_ data: [[Double]]) throws -> (rows: Int, cols: Int) {
        guard let first = data.first else {
            throw invalidArguments("Data array is empty")
        }
        let cols = first.count
        guard cols > 0, data.allSatisfy({ $0.count == cols }) else {
            throw invalidArguments("Data array must be rectangular")
        }
        return (data.count, cols)
    }
}

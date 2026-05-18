import Foundation

public enum DataFilter {
    public static func set_log_level(_ log_level: Int) throws {
        try DataFilterNative.withData { native in
            try checkBrainFlowExitCode(native.set_log_level_data_handler(CInt(log_level)), "Error in set_log_level")
        }
    }

    public static func set_log_level(_ log_level: LogLevels) throws {
        try set_log_level(log_level.rawValue)
    }

    public static func enable_data_logger() throws {
        try set_log_level(LogLevels.LEVEL_INFO)
    }

    public static func enable_dev_data_logger() throws {
        try set_log_level(LogLevels.LEVEL_TRACE)
    }

    public static func disable_data_logger() throws {
        try set_log_level(LogLevels.LEVEL_OFF)
    }

    public static func set_log_file(_ log_file: String) throws {
        try log_file.withCString { path in
            try DataFilterNative.withData { native in
                try checkBrainFlowExitCode(native.set_log_file_data_handler(path), "Error in set_log_file")
            }
        }
    }

    public static func log_message(_ log_level: Int, message: String) throws {
        var mutableMessage = Array(message.utf8CString)
        try mutableMessage.withUnsafeMutableBufferPointer { pointer in
            try DataFilterNative.withData { native in
                try checkBrainFlowExitCode(native.log_message_data_handler(CInt(log_level), pointer.baseAddress), "Error in log_message")
            }
        }
    }

    public static func get_version() throws -> String {
        try getVersion(function: \.get_version_data_handler)
    }

    public static func perform_lowpass(
        data: inout [Double],
        sampling_rate: Int,
        cutoff: Double,
        order: Int,
        filter_type: Int,
        ripple: Double
    ) throws {
        try withMutableData(&data) { pointer, count in
            try DataFilterNative.withData { native in
                try checkBrainFlowExitCode(native.perform_lowpass(pointer, CInt(count), CInt(sampling_rate), cutoff, CInt(order), CInt(filter_type), ripple), "Failed to perform lowpass")
            }
        }
    }

    public static func perform_lowpass(
        data: inout [Double],
        sampling_rate: Int,
        cutoff: Double,
        order: Int,
        filter_type: FilterTypes,
        ripple: Double
    ) throws {
        try perform_lowpass(data: &data, sampling_rate: sampling_rate, cutoff: cutoff, order: order, filter_type: filter_type.rawValue, ripple: ripple)
    }

    public static func perform_highpass(
        data: inout [Double],
        sampling_rate: Int,
        cutoff: Double,
        order: Int,
        filter_type: Int,
        ripple: Double
    ) throws {
        try withMutableData(&data) { pointer, count in
            try DataFilterNative.withData { native in
                try checkBrainFlowExitCode(native.perform_highpass(pointer, CInt(count), CInt(sampling_rate), cutoff, CInt(order), CInt(filter_type), ripple), "Failed to perform highpass")
            }
        }
    }

    public static func perform_highpass(
        data: inout [Double],
        sampling_rate: Int,
        cutoff: Double,
        order: Int,
        filter_type: FilterTypes,
        ripple: Double
    ) throws {
        try perform_highpass(data: &data, sampling_rate: sampling_rate, cutoff: cutoff, order: order, filter_type: filter_type.rawValue, ripple: ripple)
    }

    public static func perform_bandpass(
        data: inout [Double],
        sampling_rate: Int,
        start_freq: Double,
        stop_freq: Double,
        order: Int,
        filter_type: Int,
        ripple: Double
    ) throws {
        try withMutableData(&data) { pointer, count in
            try DataFilterNative.withData { native in
                try checkBrainFlowExitCode(native.perform_bandpass(pointer, CInt(count), CInt(sampling_rate), start_freq, stop_freq, CInt(order), CInt(filter_type), ripple), "Failed to perform bandpass")
            }
        }
    }

    public static func perform_bandpass(
        data: inout [Double],
        sampling_rate: Int,
        start_freq: Double,
        stop_freq: Double,
        order: Int,
        filter_type: FilterTypes,
        ripple: Double
    ) throws {
        try perform_bandpass(data: &data, sampling_rate: sampling_rate, start_freq: start_freq, stop_freq: stop_freq, order: order, filter_type: filter_type.rawValue, ripple: ripple)
    }

    public static func perform_bandstop(
        data: inout [Double],
        sampling_rate: Int,
        start_freq: Double,
        stop_freq: Double,
        order: Int,
        filter_type: Int,
        ripple: Double
    ) throws {
        try withMutableData(&data) { pointer, count in
            try DataFilterNative.withData { native in
                try checkBrainFlowExitCode(native.perform_bandstop(pointer, CInt(count), CInt(sampling_rate), start_freq, stop_freq, CInt(order), CInt(filter_type), ripple), "Failed to perform bandstop")
            }
        }
    }

    public static func perform_bandstop(
        data: inout [Double],
        sampling_rate: Int,
        start_freq: Double,
        stop_freq: Double,
        order: Int,
        filter_type: FilterTypes,
        ripple: Double
    ) throws {
        try perform_bandstop(data: &data, sampling_rate: sampling_rate, start_freq: start_freq, stop_freq: stop_freq, order: order, filter_type: filter_type.rawValue, ripple: ripple)
    }

    public static func remove_environmental_noise(data: inout [Double], sampling_rate: Int, noise_type: Int) throws {
        try withMutableData(&data) { pointer, count in
            try DataFilterNative.withData { native in
                try checkBrainFlowExitCode(native.remove_environmental_noise(pointer, CInt(count), CInt(sampling_rate), CInt(noise_type)), "Failed to remove environmental noise")
            }
        }
    }

    public static func remove_environmental_noise(data: inout [Double], sampling_rate: Int, noise_type: NoiseTypes) throws {
        try remove_environmental_noise(data: &data, sampling_rate: sampling_rate, noise_type: noise_type.rawValue)
    }

    public static func perform_rolling_filter(data: inout [Double], period: Int, operation: Int) throws {
        try withMutableData(&data) { pointer, count in
            try DataFilterNative.withData { native in
                try checkBrainFlowExitCode(native.perform_rolling_filter(pointer, CInt(count), CInt(period), CInt(operation)), "Failed to perform rolling filter")
            }
        }
    }

    public static func perform_rolling_filter(data: inout [Double], period: Int, operation: AggOperations) throws {
        try perform_rolling_filter(data: &data, period: period, operation: operation.rawValue)
    }

    public static func detrend(data: inout [Double], detrend_operation: Int) throws {
        try withMutableData(&data) { pointer, count in
            try DataFilterNative.withData { native in
                try checkBrainFlowExitCode(native.detrend(pointer, CInt(count), CInt(detrend_operation)), "Failed to detrend data")
            }
        }
    }

    public static func detrend(data: inout [Double], detrend_operation: DetrendOperations) throws {
        try detrend(data: &data, detrend_operation: detrend_operation.rawValue)
    }

    public static func perform_downsampling(data: [Double], period: Int, operation: Int) throws -> [Double] {
        guard period > 0, data.count / period > 0 else { throw invalidArguments("Invalid period or data size") }
        var input = data
        var output = [Double](repeating: 0.0, count: data.count / period)
        try input.withUnsafeMutableBufferPointer { inputPtr in
            try output.withUnsafeMutableBufferPointer { outputPtr in
                try DataFilterNative.withData { native in
                    try checkBrainFlowExitCode(native.perform_downsampling(inputPtr.baseAddress, CInt(data.count), CInt(period), CInt(operation), outputPtr.baseAddress), "Failed to perform downsampling")
                }
            }
        }
        return output
    }

    public static func perform_downsampling(data: [Double], period: Int, operation: AggOperations) throws -> [Double] {
        try perform_downsampling(data: data, period: period, operation: operation.rawValue)
    }

    public static func perform_wavelet_transform(
        data: [Double],
        wavelet: Int,
        decomposition_level: Int,
        extension_type: Int
    ) throws -> WaveletTransform {
        guard decomposition_level > 0 else { throw invalidArguments("Invalid decomposition level") }
        var input = data
        var output = [Double](repeating: 0.0, count: data.count + 2 * decomposition_level * 41)
        var lengths = [CInt](repeating: 0, count: decomposition_level + 1)
        try input.withUnsafeMutableBufferPointer { inputPtr in
            try output.withUnsafeMutableBufferPointer { outputPtr in
                try lengths.withUnsafeMutableBufferPointer { lengthsPtr in
                    try DataFilterNative.withData { native in
                        try checkBrainFlowExitCode(native.perform_wavelet_transform(inputPtr.baseAddress, CInt(data.count), CInt(wavelet), CInt(decomposition_level), CInt(extension_type), outputPtr.baseAddress, lengthsPtr.baseAddress), "Failed to perform wavelet transform")
                    }
                }
            }
        }
        let swiftLengths = lengths.map(Int.init)
        return WaveletTransform(coefficients: Array(output.prefix(swiftLengths.reduce(0, +))), decomposition_lengths: swiftLengths)
    }

    public static func perform_wavelet_transform(
        data: [Double],
        wavelet: WaveletTypes,
        decomposition_level: Int,
        extension_type: WaveletExtensionTypes
    ) throws -> WaveletTransform {
        try perform_wavelet_transform(data: data, wavelet: wavelet.rawValue, decomposition_level: decomposition_level, extension_type: extension_type.rawValue)
    }

    public static func perform_inverse_wavelet_transform(
        wavelet_output: WaveletTransform,
        original_data_len: Int,
        wavelet: Int,
        decomposition_level: Int,
        extension_type: Int
    ) throws -> [Double] {
        var coeffs = wavelet_output.coefficients
        var lengths = wavelet_output.decomposition_lengths.map(CInt.init)
        var output = [Double](repeating: 0.0, count: original_data_len)
        try coeffs.withUnsafeMutableBufferPointer { coeffsPtr in
            try lengths.withUnsafeMutableBufferPointer { lengthsPtr in
                try output.withUnsafeMutableBufferPointer { outputPtr in
                    try DataFilterNative.withData { native in
                        try checkBrainFlowExitCode(native.perform_inverse_wavelet_transform(coeffsPtr.baseAddress, CInt(original_data_len), CInt(wavelet), CInt(decomposition_level), CInt(extension_type), lengthsPtr.baseAddress, outputPtr.baseAddress), "Failed to perform inverse wavelet transform")
                    }
                }
            }
        }
        return output
    }

    public static func perform_inverse_wavelet_transform(
        wavelet_output: WaveletTransform,
        original_data_len: Int,
        wavelet: WaveletTypes,
        decomposition_level: Int,
        extension_type: WaveletExtensionTypes
    ) throws -> [Double] {
        try perform_inverse_wavelet_transform(wavelet_output: wavelet_output, original_data_len: original_data_len, wavelet: wavelet.rawValue, decomposition_level: decomposition_level, extension_type: extension_type.rawValue)
    }

    public static func perform_wavelet_denoising(
        data: inout [Double],
        wavelet: Int,
        decomposition_level: Int,
        wavelet_denoising: Int,
        threshold: Int,
        extension_type: Int,
        noise_level: Int
    ) throws {
        try withMutableData(&data) { pointer, count in
            try DataFilterNative.withData { native in
                try checkBrainFlowExitCode(native.perform_wavelet_denoising(pointer, CInt(count), CInt(wavelet), CInt(decomposition_level), CInt(wavelet_denoising), CInt(threshold), CInt(extension_type), CInt(noise_level)), "Failed to perform wavelet denoising")
            }
        }
    }

    public static func perform_wavelet_denoising(
        data: inout [Double],
        wavelet: WaveletTypes,
        decomposition_level: Int,
        wavelet_denoising: WaveletDenoisingTypes,
        threshold: ThresholdTypes,
        extension_type: WaveletExtensionTypes,
        noise_level: NoiseEstimationLevelTypes
    ) throws {
        try perform_wavelet_denoising(data: &data, wavelet: wavelet.rawValue, decomposition_level: decomposition_level, wavelet_denoising: wavelet_denoising.rawValue, threshold: threshold.rawValue, extension_type: extension_type.rawValue, noise_level: noise_level.rawValue)
    }

    public static func restore_data_from_wavelet_detailed_coeffs(
        data: [Double],
        wavelet: Int,
        decomposition_level: Int,
        level_to_restore: Int
    ) throws -> [Double] {
        var input = data
        var output = [Double](repeating: 0.0, count: data.count)
        try input.withUnsafeMutableBufferPointer { inputPtr in
            try output.withUnsafeMutableBufferPointer { outputPtr in
                try DataFilterNative.withData { native in
                    try checkBrainFlowExitCode(native.restore_data_from_wavelet_detailed_coeffs(inputPtr.baseAddress, CInt(data.count), CInt(wavelet), CInt(decomposition_level), CInt(level_to_restore), outputPtr.baseAddress), "Failed to restore wavelet detailed coeffs")
                }
            }
        }
        return output
    }

    public static func restore_data_from_wavelet_detailed_coeffs(
        data: [Double],
        wavelet: WaveletTypes,
        decomposition_level: Int,
        level_to_restore: Int
    ) throws -> [Double] {
        try restore_data_from_wavelet_detailed_coeffs(data: data, wavelet: wavelet.rawValue, decomposition_level: decomposition_level, level_to_restore: level_to_restore)
    }

    public static func detect_peaks_z_score(data: [Double], lag: Int = 5, threshold: Double = 3.5, influence: Double = 0.1) throws -> [Double] {
        var input = data
        var output = [Double](repeating: 0.0, count: data.count)
        try input.withUnsafeMutableBufferPointer { inputPtr in
            try output.withUnsafeMutableBufferPointer { outputPtr in
                try DataFilterNative.withData { native in
                    try checkBrainFlowExitCode(native.detect_peaks_z_score(inputPtr.baseAddress, CInt(data.count), CInt(lag), threshold, influence, outputPtr.baseAddress), "Failed to detect peaks")
                }
            }
        }
        return output
    }

    public static func get_csp(data: [[[Double]]], labels: [Double]) throws -> CSPResult {
        guard let firstEpoch = data.first, let firstChannel = firstEpoch.first else { throw invalidArguments("Invalid CSP data") }
        let nEpochs = data.count
        let nChannels = firstEpoch.count
        let nTimes = firstChannel.count
        var flattened = [Double]()
        flattened.reserveCapacity(nEpochs * nChannels * nTimes)
        for epoch in data {
            for channel in epoch {
                flattened.append(contentsOf: channel)
            }
        }
        var mutableLabels = labels
        var filters = [Double](repeating: 0.0, count: nChannels * nChannels)
        var eigenvalues = [Double](repeating: 0.0, count: nChannels)
        try flattened.withUnsafeMutableBufferPointer { dataPtr in
            try mutableLabels.withUnsafeMutableBufferPointer { labelsPtr in
                try filters.withUnsafeMutableBufferPointer { filtersPtr in
                    try eigenvalues.withUnsafeMutableBufferPointer { eigenPtr in
                        try DataFilterNative.withData { native in
                            try checkBrainFlowExitCode(native.get_csp(dataPtr.baseAddress, labelsPtr.baseAddress, CInt(nEpochs), CInt(nChannels), CInt(nTimes), filtersPtr.baseAddress, eigenPtr.baseAddress), "Failed to get CSP")
                        }
                    }
                }
            }
        }
        return CSPResult(filters: BrainFlowArray.reshape_data_to_2d(num_rows: nChannels, num_cols: nChannels, linear_buffer: filters), eigenvalues: eigenvalues)
    }

    public static func get_window(window_function: Int, window_len: Int) throws -> [Double] {
        var output = [Double](repeating: 0.0, count: window_len)
        try output.withUnsafeMutableBufferPointer { pointer in
            try DataFilterNative.withData { native in
                try checkBrainFlowExitCode(native.get_window(CInt(window_function), CInt(window_len), pointer.baseAddress), "Failed to get window")
            }
        }
        return output
    }

    public static func get_window(window_function: WindowOperations, window_len: Int) throws -> [Double] {
        try get_window(window_function: window_function.rawValue, window_len: window_len)
    }

    public static func perform_fft(data: [Double], start_pos: Int, end_pos: Int, window: Int) throws -> [Complex] {
        guard start_pos >= 0, end_pos <= data.count, start_pos < end_pos else { throw invalidArguments("Invalid position arguments") }
        var input = Array(data[start_pos..<end_pos])
        guard input.count % 2 == 0 else { throw invalidArguments("end_pos - start_pos must be even") }
        let inputCount = input.count
        var real = [Double](repeating: 0.0, count: inputCount / 2 + 1)
        var imag = [Double](repeating: 0.0, count: inputCount / 2 + 1)
        try input.withUnsafeMutableBufferPointer { inputPtr in
            try real.withUnsafeMutableBufferPointer { realPtr in
                try imag.withUnsafeMutableBufferPointer { imagPtr in
                    try DataFilterNative.withData { native in
                        try checkBrainFlowExitCode(native.perform_fft(inputPtr.baseAddress, CInt(inputCount), CInt(window), realPtr.baseAddress, imagPtr.baseAddress), "Failed to perform FFT")
                    }
                }
            }
        }
        return zip(real, imag).map { Complex(real: $0.0, imag: $0.1) }
    }

    public static func perform_fft(data: [Double], start_pos: Int, end_pos: Int, window: WindowOperations) throws -> [Complex] {
        try perform_fft(data: data, start_pos: start_pos, end_pos: end_pos, window: window.rawValue)
    }

    public static func perform_fft(data: [Double], window: Int) throws -> [Complex] {
        try perform_fft(data: data, start_pos: 0, end_pos: data.count, window: window)
    }

    public static func perform_fft(data: [Double], window: WindowOperations) throws -> [Complex] {
        try perform_fft(data: data, start_pos: 0, end_pos: data.count, window: window.rawValue)
    }

    public static func perform_ifft(data: [Complex]) throws -> [Double] {
        var real = data.map(\.real)
        var imag = data.map(\.imag)
        let restoredLength = (data.count - 1) * 2
        var output = [Double](repeating: 0.0, count: restoredLength)
        try real.withUnsafeMutableBufferPointer { realPtr in
            try imag.withUnsafeMutableBufferPointer { imagPtr in
                try output.withUnsafeMutableBufferPointer { outputPtr in
                    try DataFilterNative.withData { native in
                        try checkBrainFlowExitCode(native.perform_ifft(realPtr.baseAddress, imagPtr.baseAddress, CInt(restoredLength), outputPtr.baseAddress), "Failed to perform IFFT")
                    }
                }
            }
        }
        return output
    }

    public static func get_psd(data: [Double], start_pos: Int, end_pos: Int, sampling_rate: Int, window: Int) throws -> PSD {
        guard start_pos >= 0, end_pos <= data.count, start_pos < end_pos else { throw invalidArguments("Invalid position arguments") }
        var input = Array(data[start_pos..<end_pos])
        guard input.count % 2 == 0 else { throw invalidArguments("end_pos - start_pos must be even") }
        let inputCount = input.count
        var ampl = [Double](repeating: 0.0, count: inputCount / 2 + 1)
        var freq = [Double](repeating: 0.0, count: inputCount / 2 + 1)
        try input.withUnsafeMutableBufferPointer { inputPtr in
            try ampl.withUnsafeMutableBufferPointer { amplPtr in
                try freq.withUnsafeMutableBufferPointer { freqPtr in
                    try DataFilterNative.withData { native in
                        try checkBrainFlowExitCode(native.get_psd(inputPtr.baseAddress, CInt(inputCount), CInt(sampling_rate), CInt(window), amplPtr.baseAddress, freqPtr.baseAddress), "Failed to get PSD")
                    }
                }
            }
        }
        return PSD(ampl: ampl, freq: freq)
    }

    public static func get_psd(data: [Double], start_pos: Int, end_pos: Int, sampling_rate: Int, window: WindowOperations) throws -> PSD {
        try get_psd(data: data, start_pos: start_pos, end_pos: end_pos, sampling_rate: sampling_rate, window: window.rawValue)
    }

    public static func get_psd(data: [Double], sampling_rate: Int, window: Int) throws -> PSD {
        try get_psd(data: data, start_pos: 0, end_pos: data.count, sampling_rate: sampling_rate, window: window)
    }

    public static func get_psd(data: [Double], sampling_rate: Int, window: WindowOperations) throws -> PSD {
        try get_psd(data: data, start_pos: 0, end_pos: data.count, sampling_rate: sampling_rate, window: window.rawValue)
    }

    public static func get_psd_welch(data: [Double], nfft: Int, overlap: Int, sampling_rate: Int, window: Int) throws -> PSD {
        guard nfft % 2 == 0 else { throw invalidArguments("nfft must be even") }
        var input = data
        var ampl = [Double](repeating: 0.0, count: nfft / 2 + 1)
        var freq = [Double](repeating: 0.0, count: nfft / 2 + 1)
        try input.withUnsafeMutableBufferPointer { inputPtr in
            try ampl.withUnsafeMutableBufferPointer { amplPtr in
                try freq.withUnsafeMutableBufferPointer { freqPtr in
                    try DataFilterNative.withData { native in
                        try checkBrainFlowExitCode(native.get_psd_welch(inputPtr.baseAddress, CInt(data.count), CInt(nfft), CInt(overlap), CInt(sampling_rate), CInt(window), amplPtr.baseAddress, freqPtr.baseAddress), "Failed to get PSD Welch")
                    }
                }
            }
        }
        return PSD(ampl: ampl, freq: freq)
    }

    public static func get_psd_welch(data: [Double], nfft: Int, overlap: Int, sampling_rate: Int, window: WindowOperations) throws -> PSD {
        try get_psd_welch(data: data, nfft: nfft, overlap: overlap, sampling_rate: sampling_rate, window: window.rawValue)
    }

    public static func get_band_power(psd: PSD, freq_start: Double, freq_end: Double) throws -> Double {
        var ampl = psd.ampl
        var freq = psd.freq
        var output = 0.0
        try ampl.withUnsafeMutableBufferPointer { amplPtr in
            try freq.withUnsafeMutableBufferPointer { freqPtr in
                try DataFilterNative.withData { native in
                    try checkBrainFlowExitCode(native.get_band_power(amplPtr.baseAddress, freqPtr.baseAddress, CInt(psd.ampl.count), freq_start, freq_end, &output), "Failed to get band power")
                }
            }
        }
        return output
    }

    public static func get_avg_band_powers(data: [[Double]], channels: [Int], sampling_rate: Int, apply_filter: Bool) throws -> BandPowerResult {
        let defaultBands = [
            FrequencyBand(start: 2.0, stop: 4.0),
            FrequencyBand(start: 4.0, stop: 8.0),
            FrequencyBand(start: 8.0, stop: 13.0),
            FrequencyBand(start: 13.0, stop: 30.0),
            FrequencyBand(start: 30.0, stop: 45.0)
        ]
        return try get_custom_band_powers(data: data, bands: defaultBands, channels: channels, sampling_rate: sampling_rate, apply_filter: apply_filter)
    }

    public static func get_custom_band_powers(
        data: [[Double]],
        bands: [FrequencyBand],
        channels: [Int],
        sampling_rate: Int,
        apply_filter: Bool
    ) throws -> BandPowerResult {
        guard !channels.isEmpty, !bands.isEmpty else { throw invalidArguments("Channels and bands must be non-empty") }
        let cols = data[channels[0]].count
        var selected = [Double]()
        selected.reserveCapacity(channels.count * cols)
        for channel in channels {
            selected.append(contentsOf: data[channel])
        }
        var starts = bands.map(\.start)
        var stops = bands.map(\.stop)
        var avg = [Double](repeating: 0.0, count: bands.count)
        var stddev = [Double](repeating: 0.0, count: bands.count)
        try selected.withUnsafeMutableBufferPointer { dataPtr in
            try starts.withUnsafeMutableBufferPointer { startsPtr in
                try stops.withUnsafeMutableBufferPointer { stopsPtr in
                    try avg.withUnsafeMutableBufferPointer { avgPtr in
                        try stddev.withUnsafeMutableBufferPointer { stddevPtr in
                            try DataFilterNative.withData { native in
                                try checkBrainFlowExitCode(native.get_custom_band_powers(dataPtr.baseAddress, CInt(channels.count), CInt(cols), startsPtr.baseAddress, stopsPtr.baseAddress, CInt(bands.count), CInt(sampling_rate), apply_filter ? 1 : 0, avgPtr.baseAddress, stddevPtr.baseAddress), "Failed to get custom band powers")
                            }
                        }
                    }
                }
            }
        }
        return BandPowerResult(average: avg, stddev: stddev)
    }

    public static func perform_ica(data: [[Double]], num_components: Int, channels: [Int]? = nil) throws -> ICAResult {
        let (rows, cols) = try BrainFlowArray.validateRectangular(data)
        let selectedChannels = channels ?? Array(0..<rows)
        guard num_components >= 1 else { throw invalidArguments("num_components must be positive") }
        var selected = [Double]()
        selected.reserveCapacity(selectedChannels.count * cols)
        for channel in selectedChannels {
            selected.append(contentsOf: data[channel])
        }
        var w = [Double](repeating: 0.0, count: num_components * num_components)
        var k = [Double](repeating: 0.0, count: selectedChannels.count * num_components)
        var a = [Double](repeating: 0.0, count: num_components * selectedChannels.count)
        var s = [Double](repeating: 0.0, count: cols * num_components)
        try selected.withUnsafeMutableBufferPointer { dataPtr in
            try w.withUnsafeMutableBufferPointer { wPtr in
                try k.withUnsafeMutableBufferPointer { kPtr in
                    try a.withUnsafeMutableBufferPointer { aPtr in
                        try s.withUnsafeMutableBufferPointer { sPtr in
                            try DataFilterNative.withData { native in
                                try checkBrainFlowExitCode(native.perform_ica(dataPtr.baseAddress, CInt(selectedChannels.count), CInt(cols), CInt(num_components), wPtr.baseAddress, kPtr.baseAddress, aPtr.baseAddress, sPtr.baseAddress), "Failed to perform ICA")
                            }
                        }
                    }
                }
            }
        }
        return ICAResult(
            w: BrainFlowArray.reshape_data_to_2d(num_rows: num_components, num_cols: num_components, linear_buffer: w),
            k: BrainFlowArray.reshape_data_to_2d(num_rows: num_components, num_cols: selectedChannels.count, linear_buffer: k),
            a: BrainFlowArray.reshape_data_to_2d(num_rows: selectedChannels.count, num_cols: num_components, linear_buffer: a),
            s: BrainFlowArray.reshape_data_to_2d(num_rows: num_components, num_cols: cols, linear_buffer: s)
        )
    }

    public static func calc_stddev(data: [Double], start_pos: Int? = nil, end_pos: Int? = nil) throws -> Double {
        var input = data
        let start = start_pos ?? 0
        let end = end_pos ?? data.count
        var output = 0.0
        try input.withUnsafeMutableBufferPointer { pointer in
            try DataFilterNative.withData { native in
                try checkBrainFlowExitCode(native.calc_stddev(pointer.baseAddress, CInt(start), CInt(end), &output), "Failed to calc stddev")
            }
        }
        return output
    }

    public static func get_railed_percentage(data: [Double], gain: Int) throws -> Double {
        var input = data
        var output = 0.0
        try input.withUnsafeMutableBufferPointer { pointer in
            try DataFilterNative.withData { native in
                try checkBrainFlowExitCode(native.get_railed_percentage(pointer.baseAddress, CInt(data.count), CInt(gain), &output), "Failed to get railed percentage")
            }
        }
        return output
    }

    public static func get_oxygen_level(ppg_ir: [Double], ppg_red: [Double], sampling_rate: Int, coef1: Double = 1.5958422, coef2: Double = -34.6596622, coef3: Double = 112.6898759) throws -> Double {
        var ir = ppg_ir
        var red = ppg_red
        var output = 0.0
        try ir.withUnsafeMutableBufferPointer { irPtr in
            try red.withUnsafeMutableBufferPointer { redPtr in
                try DataFilterNative.withData { native in
                    try checkBrainFlowExitCode(native.get_oxygen_level(irPtr.baseAddress, redPtr.baseAddress, CInt(ppg_ir.count), CInt(sampling_rate), coef1, coef2, coef3, &output), "Failed to get oxygen level")
                }
            }
        }
        return output
    }

    public static func get_heart_rate(ppg_ir: [Double], ppg_red: [Double], sampling_rate: Int, fft_size: Int) throws -> Double {
        var ir = ppg_ir
        var red = ppg_red
        var output = 0.0
        try ir.withUnsafeMutableBufferPointer { irPtr in
            try red.withUnsafeMutableBufferPointer { redPtr in
                try DataFilterNative.withData { native in
                    try checkBrainFlowExitCode(native.get_heart_rate(irPtr.baseAddress, redPtr.baseAddress, CInt(ppg_ir.count), CInt(sampling_rate), CInt(fft_size), &output), "Failed to get heart rate")
                }
            }
        }
        return output
    }

    public static func get_nearest_power_of_two(_ value: Int) throws -> Int {
        var output: CInt = 0
        try DataFilterNative.withData { native in
            try checkBrainFlowExitCode(native.get_nearest_power_of_two(CInt(value), &output), "Failed to get nearest power of two")
        }
        return Int(output)
    }

    public static func write_file(data: [[Double]], file_name: String, file_mode: String) throws {
        let (rows, cols) = try BrainFlowArray.validateRectangular(data)
        var linear = reshape_data_to_1d(num_rows: rows, num_cols: cols, buf: data)
        try file_name.withCString { fileNamePtr in
            try file_mode.withCString { fileModePtr in
                try linear.withUnsafeMutableBufferPointer { linearPtr in
                    try DataFilterNative.withData { native in
                        try checkBrainFlowExitCode(native.write_file(linearPtr.baseAddress, CInt(rows), CInt(cols), fileNamePtr, fileModePtr), "Failed to write file")
                    }
                }
            }
        }
    }

    public static func read_file(_ file_name: String) throws -> [[Double]] {
        var elements: CInt = 0
        try file_name.withCString { fileNamePtr in
            try DataFilterNative.withData { native in
                try checkBrainFlowExitCode(native.get_num_elements_in_file(fileNamePtr, &elements), "Failed to determine number of file elements")
            }
        }
        var data = [Double](repeating: 0.0, count: Int(elements))
        var rows: CInt = 0
        var cols: CInt = 0
        try file_name.withCString { fileNamePtr in
            try data.withUnsafeMutableBufferPointer { dataPtr in
                try DataFilterNative.withData { native in
                    try checkBrainFlowExitCode(native.read_file(dataPtr.baseAddress, &rows, &cols, fileNamePtr, elements), "Failed to read file")
                }
            }
        }
        return BrainFlowArray.reshape_data_to_2d(num_rows: Int(rows), num_cols: Int(cols), linear_buffer: data)
    }

    public static func reshape_data_to_1d(num_rows: Int, num_cols: Int, buf: [[Double]]) -> [Double] {
        BrainFlowArray.reshape_data_to_1d(num_rows: num_rows, num_cols: num_cols, buf: buf)
    }

    public static func reshape_data_to_2d(num_rows: Int, num_cols: Int, linear_buffer: [Double]) -> [[Double]] {
        BrainFlowArray.reshape_data_to_2d(num_rows: num_rows, num_cols: num_cols, linear_buffer: linear_buffer)
    }

    private static func withMutableData<T>(_ data: inout [Double], _ body: (UnsafeMutablePointer<Double>?, Int) throws -> T) throws -> T {
        try data.withUnsafeMutableBufferPointer { pointer in
            try body(pointer.baseAddress, pointer.count)
        }
    }

    private static func getVersion(function: KeyPath<DataFilterNative, DataFilterNative.VersionFunction>) throws -> String {
        let maxLength = 64
        var bytes = [CChar](repeating: 0, count: maxLength)
        var length: CInt = 0
        try bytes.withUnsafeMutableBufferPointer { pointer in
            try DataFilterNative.withData { native in
                try checkBrainFlowExitCode(native[keyPath: function](pointer.baseAddress, &length, CInt(maxLength)), "Error in get_version")
            }
        }
        return String(bytes: bytes.prefix(Int(length)).map { UInt8(bitPattern: $0) }, encoding: .utf8) ?? ""
    }
}

final class DataFilterNative {
    typealias VersionFunction = @convention(c) (UnsafeMutablePointer<CChar>?, UnsafeMutablePointer<CInt>?, CInt) -> CInt

    let perform_lowpass: @convention(c) (UnsafeMutablePointer<Double>?, CInt, CInt, Double, CInt, CInt, Double) -> CInt
    let perform_highpass: @convention(c) (UnsafeMutablePointer<Double>?, CInt, CInt, Double, CInt, CInt, Double) -> CInt
    let perform_bandpass: @convention(c) (UnsafeMutablePointer<Double>?, CInt, CInt, Double, Double, CInt, CInt, Double) -> CInt
    let perform_bandstop: @convention(c) (UnsafeMutablePointer<Double>?, CInt, CInt, Double, Double, CInt, CInt, Double) -> CInt
    let remove_environmental_noise: @convention(c) (UnsafeMutablePointer<Double>?, CInt, CInt, CInt) -> CInt
    let perform_rolling_filter: @convention(c) (UnsafeMutablePointer<Double>?, CInt, CInt, CInt) -> CInt
    let perform_downsampling: @convention(c) (UnsafeMutablePointer<Double>?, CInt, CInt, CInt, UnsafeMutablePointer<Double>?) -> CInt
    let perform_wavelet_transform: @convention(c) (UnsafeMutablePointer<Double>?, CInt, CInt, CInt, CInt, UnsafeMutablePointer<Double>?, UnsafeMutablePointer<CInt>?) -> CInt
    let perform_inverse_wavelet_transform: @convention(c) (UnsafeMutablePointer<Double>?, CInt, CInt, CInt, CInt, UnsafeMutablePointer<CInt>?, UnsafeMutablePointer<Double>?) -> CInt
    let perform_wavelet_denoising: @convention(c) (UnsafeMutablePointer<Double>?, CInt, CInt, CInt, CInt, CInt, CInt, CInt) -> CInt
    let get_csp: @convention(c) (UnsafePointer<Double>?, UnsafePointer<Double>?, CInt, CInt, CInt, UnsafeMutablePointer<Double>?, UnsafeMutablePointer<Double>?) -> CInt
    let get_window: @convention(c) (CInt, CInt, UnsafeMutablePointer<Double>?) -> CInt
    let perform_fft: @convention(c) (UnsafeMutablePointer<Double>?, CInt, CInt, UnsafeMutablePointer<Double>?, UnsafeMutablePointer<Double>?) -> CInt
    let perform_ifft: @convention(c) (UnsafeMutablePointer<Double>?, UnsafeMutablePointer<Double>?, CInt, UnsafeMutablePointer<Double>?) -> CInt
    let get_nearest_power_of_two: @convention(c) (CInt, UnsafeMutablePointer<CInt>?) -> CInt
    let get_psd: @convention(c) (UnsafeMutablePointer<Double>?, CInt, CInt, CInt, UnsafeMutablePointer<Double>?, UnsafeMutablePointer<Double>?) -> CInt
    let detrend: @convention(c) (UnsafeMutablePointer<Double>?, CInt, CInt) -> CInt
    let calc_stddev: @convention(c) (UnsafeMutablePointer<Double>?, CInt, CInt, UnsafeMutablePointer<Double>?) -> CInt
    let get_psd_welch: @convention(c) (UnsafeMutablePointer<Double>?, CInt, CInt, CInt, CInt, CInt, UnsafeMutablePointer<Double>?, UnsafeMutablePointer<Double>?) -> CInt
    let get_band_power: @convention(c) (UnsafeMutablePointer<Double>?, UnsafeMutablePointer<Double>?, CInt, Double, Double, UnsafeMutablePointer<Double>?) -> CInt
    let get_custom_band_powers: @convention(c) (UnsafeMutablePointer<Double>?, CInt, CInt, UnsafeMutablePointer<Double>?, UnsafeMutablePointer<Double>?, CInt, CInt, CInt, UnsafeMutablePointer<Double>?, UnsafeMutablePointer<Double>?) -> CInt
    let get_railed_percentage: @convention(c) (UnsafeMutablePointer<Double>?, CInt, CInt, UnsafeMutablePointer<Double>?) -> CInt
    let get_oxygen_level: @convention(c) (UnsafeMutablePointer<Double>?, UnsafeMutablePointer<Double>?, CInt, CInt, Double, Double, Double, UnsafeMutablePointer<Double>?) -> CInt
    let get_heart_rate: @convention(c) (UnsafeMutablePointer<Double>?, UnsafeMutablePointer<Double>?, CInt, CInt, CInt, UnsafeMutablePointer<Double>?) -> CInt
    let restore_data_from_wavelet_detailed_coeffs: @convention(c) (UnsafeMutablePointer<Double>?, CInt, CInt, CInt, CInt, UnsafeMutablePointer<Double>?) -> CInt
    let detect_peaks_z_score: @convention(c) (UnsafeMutablePointer<Double>?, CInt, CInt, Double, Double, UnsafeMutablePointer<Double>?) -> CInt
    let perform_ica: @convention(c) (UnsafeMutablePointer<Double>?, CInt, CInt, CInt, UnsafeMutablePointer<Double>?, UnsafeMutablePointer<Double>?, UnsafeMutablePointer<Double>?, UnsafeMutablePointer<Double>?) -> CInt
    let set_log_level_data_handler: @convention(c) (CInt) -> CInt
    let set_log_file_data_handler: @convention(c) (UnsafePointer<CChar>?) -> CInt
    let log_message_data_handler: @convention(c) (CInt, UnsafeMutablePointer<CChar>?) -> CInt
    let write_file: @convention(c) (UnsafePointer<Double>?, CInt, CInt, UnsafePointer<CChar>?, UnsafePointer<CChar>?) -> CInt
    let read_file: @convention(c) (UnsafeMutablePointer<Double>?, UnsafeMutablePointer<CInt>?, UnsafeMutablePointer<CInt>?, UnsafePointer<CChar>?, CInt) -> CInt
    let get_num_elements_in_file: @convention(c) (UnsafePointer<CChar>?, UnsafeMutablePointer<CInt>?) -> CInt
    let get_version_data_handler: VersionFunction

    private static let lock = NSLock()
    private static var cached: DataFilterNative?

    static func withData<T>(_ body: (DataFilterNative) throws -> T) throws -> T {
        try body(load())
    }

    private static func load() throws -> DataFilterNative {
        lock.lock()
        defer { lock.unlock() }
        if let cached { return cached }
        let value = try DataFilterNative(library: NativeLibraries.dataHandler.load())
        cached = value
        return value
    }

    private init(library: NativeLibrary) throws {
        perform_lowpass = try library.symbol("perform_lowpass", as: type(of: perform_lowpass))
        perform_highpass = try library.symbol("perform_highpass", as: type(of: perform_highpass))
        perform_bandpass = try library.symbol("perform_bandpass", as: type(of: perform_bandpass))
        perform_bandstop = try library.symbol("perform_bandstop", as: type(of: perform_bandstop))
        remove_environmental_noise = try library.symbol("remove_environmental_noise", as: type(of: remove_environmental_noise))
        perform_rolling_filter = try library.symbol("perform_rolling_filter", as: type(of: perform_rolling_filter))
        perform_downsampling = try library.symbol("perform_downsampling", as: type(of: perform_downsampling))
        perform_wavelet_transform = try library.symbol("perform_wavelet_transform", as: type(of: perform_wavelet_transform))
        perform_inverse_wavelet_transform = try library.symbol("perform_inverse_wavelet_transform", as: type(of: perform_inverse_wavelet_transform))
        perform_wavelet_denoising = try library.symbol("perform_wavelet_denoising", as: type(of: perform_wavelet_denoising))
        get_csp = try library.symbol("get_csp", as: type(of: get_csp))
        get_window = try library.symbol("get_window", as: type(of: get_window))
        perform_fft = try library.symbol("perform_fft", as: type(of: perform_fft))
        perform_ifft = try library.symbol("perform_ifft", as: type(of: perform_ifft))
        get_nearest_power_of_two = try library.symbol("get_nearest_power_of_two", as: type(of: get_nearest_power_of_two))
        get_psd = try library.symbol("get_psd", as: type(of: get_psd))
        detrend = try library.symbol("detrend", as: type(of: detrend))
        calc_stddev = try library.symbol("calc_stddev", as: type(of: calc_stddev))
        get_psd_welch = try library.symbol("get_psd_welch", as: type(of: get_psd_welch))
        get_band_power = try library.symbol("get_band_power", as: type(of: get_band_power))
        get_custom_band_powers = try library.symbol("get_custom_band_powers", as: type(of: get_custom_band_powers))
        get_railed_percentage = try library.symbol("get_railed_percentage", as: type(of: get_railed_percentage))
        get_oxygen_level = try library.symbol("get_oxygen_level", as: type(of: get_oxygen_level))
        get_heart_rate = try library.symbol("get_heart_rate", as: type(of: get_heart_rate))
        restore_data_from_wavelet_detailed_coeffs = try library.symbol("restore_data_from_wavelet_detailed_coeffs", as: type(of: restore_data_from_wavelet_detailed_coeffs))
        detect_peaks_z_score = try library.symbol("detect_peaks_z_score", as: type(of: detect_peaks_z_score))
        perform_ica = try library.symbol("perform_ica", as: type(of: perform_ica))
        set_log_level_data_handler = try library.symbol("set_log_level_data_handler", as: type(of: set_log_level_data_handler))
        set_log_file_data_handler = try library.symbol("set_log_file_data_handler", as: type(of: set_log_file_data_handler))
        log_message_data_handler = try library.symbol("log_message_data_handler", as: type(of: log_message_data_handler))
        write_file = try library.symbol("write_file", as: type(of: write_file))
        read_file = try library.symbol("read_file", as: type(of: read_file))
        get_num_elements_in_file = try library.symbol("get_num_elements_in_file", as: type(of: get_num_elements_in_file))
        get_version_data_handler = try library.symbol("get_version_data_handler", as: type(of: get_version_data_handler))
    }
}

import BrainFlow

let data = Array(0..<256).map { sin(Double($0) / 10.0) }
let psd = try DataFilter.get_psd(data: data, start_pos: 0, end_pos: data.count, sampling_rate: 250, window: WindowOperations.HANNING.rawValue)
let alpha = try DataFilter.get_band_power(psd: psd, freq_start: 8.0, freq_end: 13.0)
print("Alpha power: \(alpha)")

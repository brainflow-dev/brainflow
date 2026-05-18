import BrainFlow

let data = Array(0..<256).map { sin(Double($0) / 10.0) }
let fft = try DataFilter.perform_fft(data: data, start_pos: 0, end_pos: data.count, window: WindowOperations.HANNING)
let restored = try DataFilter.perform_ifft(data: fft)
print("FFT bins: \(fft.count)")
print("Restored samples: \(restored.count)")

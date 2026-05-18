import BrainFlow

var data = Array(0..<256).map { sin(Double($0) / 10.0) }
try DataFilter.perform_lowpass(data: &data, sampling_rate: 250, cutoff: 30.0, order: 4, filter_type: FilterTypes.BUTTERWORTH, ripple: 0.0)
try DataFilter.perform_highpass(data: &data, sampling_rate: 250, cutoff: 1.0, order: 4, filter_type: FilterTypes.BUTTERWORTH, ripple: 0.0)
print(data.prefix(10))

import BrainFlow
import BrainFlowExampleSupport

@main
enum SignalFilteringExample {
    static func main() throws {
        let sample = try SyntheticBoardDataReader.read(maxSamples: 256)
        var data = try sample.firstEEGChannel()
        try DataFilter.perform_lowpass(data: &data, sampling_rate: sample.samplingRate, cutoff: 30.0, order: 4, filter_type: FilterTypes.BUTTERWORTH, ripple: 0.0)
        try DataFilter.perform_highpass(data: &data, sampling_rate: sample.samplingRate, cutoff: 1.0, order: 4, filter_type: FilterTypes.BUTTERWORTH, ripple: 0.0)
        print(data.prefix(10))
    }
}

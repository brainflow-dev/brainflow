import BrainFlow
import BrainFlowExampleSupport

@main
enum BandPowerExample {
    static func main() throws {
        let sample = try SyntheticBoardDataReader.read(maxSamples: 256)
        let data = try sample.firstEEGChannel()
        let psd = try DataFilter.get_psd(
            data: data,
            start_pos: 0,
            end_pos: data.count,
            sampling_rate: sample.samplingRate,
            window: WindowOperations.HANNING.rawValue
        )
        let alpha = try DataFilter.get_band_power(psd: psd, freq_start: 8.0, freq_end: 13.0)
        print("Alpha power: \(alpha)")
    }
}

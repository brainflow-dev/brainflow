import BrainFlow
import BrainFlowExampleSupport

@main
enum TransformsExample {
    static func main() throws {
        let sample = try SyntheticBoardDataReader.read(maxSamples: 256)
        let data = try sample.firstEEGChannel()
        let fft = try DataFilter.perform_fft(data: data, start_pos: 0, end_pos: data.count, window: WindowOperations.HANNING)
        let restored = try DataFilter.perform_ifft(data: fft)
        print("FFT bins: \(fft.count)")
        print("Restored samples: \(restored.count)")
    }
}

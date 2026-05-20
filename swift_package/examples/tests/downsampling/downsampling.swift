import BrainFlow
import BrainFlowExampleSupport

@main
enum DownsamplingExample {
    static func main() throws {
        let sample = try SyntheticBoardDataReader.read(maxSamples: 256)
        let data = try sample.firstEEGChannel()
        let downsampled = try DataFilter.perform_downsampling(data: data, period: 4, operation: AggOperations.MEAN)
        print(downsampled)
    }
}

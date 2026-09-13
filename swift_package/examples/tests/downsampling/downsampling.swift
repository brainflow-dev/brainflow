import BrainFlow
import BrainFlowExampleSupport

@main
enum DownsamplingExample {
    static func main() throws {
        let sample = try SyntheticBoardDataReader.read(maxSamples: 256)
        var referencedData = sample.data
        if sample.eegChannels.count >= 2 {
            // Re-reference every EEG channel using the sample-wise mean of the first two EEG channels.
            // The operation changes referencedData in-place.
            try DataFilter.reference(
                data: &referencedData,
                channels_to_reference: sample.eegChannels,
                reference_channels: Array(sample.eegChannels.prefix(2))
            )
        }

        guard let firstEEGChannel = sample.eegChannels.first else {
            throw BrainFlowError("No EEG channel found", BrainFlowExitCodes.GENERAL_ERROR.rawValue)
        }
        let downsampled = try DataFilter.perform_downsampling(
            data: referencedData[firstEEGChannel],
            period: 4,
            operation: AggOperations.MEAN
        )
        print(downsampled)
    }
}

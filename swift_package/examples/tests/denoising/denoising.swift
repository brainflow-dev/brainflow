import BrainFlow
import BrainFlowExampleSupport

@main
enum DenoisingExample {
    static func main() throws {
        let sample = try SyntheticBoardDataReader.read(maxSamples: 256)
        var data = try sample.firstEEGChannel()
        try DataFilter.perform_wavelet_denoising(
            data: &data,
            wavelet: WaveletTypes.DB5,
            decomposition_level: 3,
            wavelet_denoising: WaveletDenoisingTypes.SURESHRINK,
            threshold: ThresholdTypes.HARD,
            extension_type: WaveletExtensionTypes.SYMMETRIC,
            noise_level: NoiseEstimationLevelTypes.FIRST_LEVEL
        )
        print(data.prefix(10))
    }
}

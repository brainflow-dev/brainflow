import BrainFlow

var data = Array(0..<256).map { sin(Double($0) / 10.0) }
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

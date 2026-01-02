//! DataFilter - Signal processing utilities.

use alloc::vec::Vec;

use brainflow_fft::{self, WindowType};
use brainflow_filter::{self, DetrendType, FilterType, NoiseType};
use brainflow_math::ica::{perform_ica, FastIcaParams};
use brainflow_wavelet::{self, DenoiseMethod, ThresholdType, Wavelet};

/// Signal processing utilities matching the C++ DataFilter API.
pub struct DataFilter;

impl DataFilter {
    // ========================
    // Filtering
    // ========================

    /// Perform low-pass filtering.
    pub fn perform_lowpass(
        data: &mut [f64],
        sampling_rate: f64,
        cutoff: f64,
        order: usize,
        filter_type: FilterType,
        ripple: f64,
    ) {
        brainflow_filter::perform_lowpass(data, sampling_rate, cutoff, order, filter_type, ripple);
    }

    /// Perform high-pass filtering.
    pub fn perform_highpass(
        data: &mut [f64],
        sampling_rate: f64,
        cutoff: f64,
        order: usize,
        filter_type: FilterType,
        ripple: f64,
    ) {
        brainflow_filter::perform_highpass(data, sampling_rate, cutoff, order, filter_type, ripple);
    }

    /// Perform band-pass filtering.
    pub fn perform_bandpass(
        data: &mut [f64],
        sampling_rate: f64,
        low_cutoff: f64,
        high_cutoff: f64,
        order: usize,
        filter_type: FilterType,
        ripple: f64,
    ) {
        brainflow_filter::perform_bandpass(
            data,
            sampling_rate,
            low_cutoff,
            high_cutoff,
            order,
            filter_type,
            ripple,
        );
    }

    /// Perform band-stop filtering.
    pub fn perform_bandstop(
        data: &mut [f64],
        sampling_rate: f64,
        low_cutoff: f64,
        high_cutoff: f64,
        order: usize,
        filter_type: FilterType,
        ripple: f64,
    ) {
        brainflow_filter::perform_bandstop(
            data,
            sampling_rate,
            low_cutoff,
            high_cutoff,
            order,
            filter_type,
            ripple,
        );
    }

    /// Remove environmental noise (50/60 Hz).
    pub fn remove_environmental_noise(
        data: &mut [f64],
        sampling_rate: f64,
        noise_type: NoiseType,
    ) {
        brainflow_filter::remove_environmental_noise(data, sampling_rate, noise_type);
    }

    /// Detrend a signal.
    pub fn detrend(data: &mut [f64], detrend_type: DetrendType) {
        brainflow_filter::detrend(data, detrend_type);
    }

    // ========================
    // FFT / Spectral Analysis
    // ========================

    /// Perform FFT.
    ///
    /// Returns (real, imaginary) components.
    pub fn perform_fft(data: &[f64], window: WindowType) -> (Vec<f64>, Vec<f64>) {
        brainflow_fft::perform_fft(data, window)
    }

    /// Perform inverse FFT.
    pub fn perform_ifft(real: &[f64], imag: &[f64]) -> Vec<f64> {
        brainflow_fft::perform_ifft(real, imag)
    }

    /// Get Power Spectral Density.
    ///
    /// Returns (psd, frequencies).
    pub fn get_psd(
        data: &[f64],
        sampling_rate: f64,
        window: WindowType,
    ) -> (Vec<f64>, Vec<f64>) {
        brainflow_fft::get_psd(data, sampling_rate, window)
    }

    /// Get PSD using Welch's method.
    ///
    /// Returns (psd, frequencies).
    pub fn get_psd_welch(
        data: &[f64],
        nfft: usize,
        overlap: usize,
        sampling_rate: f64,
        window: WindowType,
    ) -> (Vec<f64>, Vec<f64>) {
        brainflow_fft::get_psd_welch(data, nfft, overlap, sampling_rate, window)
    }

    /// Get band power in a frequency range.
    pub fn get_band_power(
        psd: &[f64],
        frequencies: &[f64],
        freq_start: f64,
        freq_end: f64,
    ) -> f64 {
        brainflow_fft::get_band_power(psd, frequencies, freq_start, freq_end)
    }

    /// Get nearest power of 2.
    pub fn get_nearest_power_of_two(n: usize) -> usize {
        brainflow_fft::nearest_power_of_two(n)
    }

    // ========================
    // Wavelet Transforms
    // ========================

    /// Perform wavelet transform.
    ///
    /// Returns (coefficients, lengths).
    pub fn perform_wavelet_transform(
        data: &[f64],
        wavelet: Wavelet,
        level: usize,
    ) -> (Vec<f64>, Vec<usize>) {
        brainflow_wavelet::perform_wavelet_transform(data, wavelet, level)
    }

    /// Perform inverse wavelet transform.
    pub fn perform_inverse_wavelet_transform(
        coefficients: &[f64],
        lengths: &[usize],
        wavelet: Wavelet,
    ) -> Vec<f64> {
        brainflow_wavelet::perform_inverse_wavelet_transform(coefficients, lengths, wavelet)
    }

    /// Perform wavelet denoising.
    pub fn perform_wavelet_denoising(
        data: &mut [f64],
        wavelet: Wavelet,
        level: usize,
    ) {
        brainflow_wavelet::perform_wavelet_denoising(
            data,
            wavelet,
            level,
            DenoiseMethod::VisuShrink,
            ThresholdType::Soft,
        );
    }

    // ========================
    // ICA
    // ========================

    /// Perform Independent Component Analysis.
    ///
    /// # Arguments
    /// * `data` - Flattened data (num_channels × num_samples).
    /// * `num_channels` - Number of channels.
    /// * `num_samples` - Number of samples.
    /// * `num_components` - Number of independent components to extract.
    ///
    /// # Returns
    /// (unmixing_matrix, mixing_matrix, whitening_matrix, sources)
    pub fn perform_ica(
        data: &[f64],
        num_channels: usize,
        num_samples: usize,
        num_components: usize,
    ) -> Option<(Vec<f64>, Vec<f64>, Vec<f64>, Vec<f64>)> {
        brainflow_math::ica::perform_ica(data, num_channels, num_samples, num_components)
    }

    // ========================
    // Downsampling
    // ========================

    /// Downsample data using mean.
    pub fn perform_downsampling(data: &[f64], period: usize) -> Vec<f64> {
        if period == 0 || data.is_empty() {
            return Vec::new();
        }

        data.chunks(period)
            .map(|chunk| chunk.iter().sum::<f64>() / chunk.len() as f64)
            .collect()
    }

    // ========================
    // Rolling Filters
    // ========================

    /// Apply rolling mean filter.
    pub fn perform_rolling_filter_mean(data: &mut [f64], window_size: usize) {
        let filtered = brainflow_filter::iir::rolling::apply_rolling_average(data, window_size);
        data[..filtered.len()].copy_from_slice(&filtered);
    }

    /// Apply rolling median filter.
    pub fn perform_rolling_filter_median(data: &mut [f64], window_size: usize) {
        let filtered = brainflow_filter::iir::rolling::apply_rolling_median(data, window_size);
        data[..filtered.len()].copy_from_slice(&filtered);
    }

    // ========================
    // Statistics
    // ========================

    /// Calculate mean of data.
    pub fn calc_mean(data: &[f64]) -> f64 {
        brainflow_math::stats::mean(data)
    }

    /// Calculate standard deviation.
    pub fn calc_stddev(data: &[f64]) -> f64 {
        brainflow_math::stats::stddev(data)
    }

    /// Detect peaks using z-score algorithm.
    pub fn detect_peaks_z_score(
        data: &[f64],
        lag: usize,
        threshold: f64,
        influence: f64,
    ) -> Vec<i32> {
        brainflow_math::stats::detect_peaks_z_score(data, lag, threshold, influence)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_lowpass() {
        let mut data: Vec<f64> = (0..100)
            .map(|i| libm::sin(0.1 * i as f64) + libm::sin(0.5 * i as f64))
            .collect();

        DataFilter::perform_lowpass(&mut data, 100.0, 10.0, 4, FilterType::Butterworth, 0.0);

        // Just verify it runs without panic
        assert_eq!(data.len(), 100);
    }

    #[test]
    fn test_fft() {
        let data: Vec<f64> = (0..64)
            .map(|i| libm::sin(2.0 * core::f64::consts::PI * 5.0 * i as f64 / 64.0))
            .collect();

        let (real, imag) = DataFilter::perform_fft(&data, WindowType::Hanning);

        assert!(!real.is_empty());
        assert!(!imag.is_empty());
    }

    #[test]
    fn test_psd() {
        let data: Vec<f64> = (0..256)
            .map(|i| libm::sin(2.0 * core::f64::consts::PI * 10.0 * i as f64 / 256.0))
            .collect();

        let (psd, freqs) = DataFilter::get_psd(&data, 256.0, WindowType::Hanning);

        assert!(!psd.is_empty());
        assert!(!freqs.is_empty());
        assert_eq!(psd.len(), freqs.len());
    }

    #[test]
    fn test_wavelet() {
        let data: Vec<f64> = (0..64).map(|i| i as f64).collect();

        let (coeffs, lengths) = DataFilter::perform_wavelet_transform(&data, Wavelet::Haar, 3);

        let recovered =
            DataFilter::perform_inverse_wavelet_transform(&coeffs, &lengths, Wavelet::Haar);

        assert_eq!(recovered.len(), data.len());
    }

    #[test]
    fn test_downsampling() {
        let data = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0];
        let downsampled = DataFilter::perform_downsampling(&data, 2);

        assert_eq!(downsampled.len(), 3);
        assert!((downsampled[0] - 1.5).abs() < 1e-10);
        assert!((downsampled[1] - 3.5).abs() < 1e-10);
        assert!((downsampled[2] - 5.5).abs() < 1e-10);
    }
}

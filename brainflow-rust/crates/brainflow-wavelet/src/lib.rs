//! Pure Rust wavelet transform implementation for BrainFlow.
//!
//! This crate provides:
//! - Discrete Wavelet Transform (DWT) and inverse (IDWT)
//! - Multiple wavelet families (Haar, Daubechies, Symlet, Coiflet, Biorthogonal)
//! - Wavelet denoising (VisuShrink, SureShrink)

#![no_std]

extern crate alloc;

use alloc::vec::Vec;

pub mod coefficients;
pub mod denoise;
pub mod transform;
pub mod wavelets;

pub use coefficients::WaveletCoefficients;
pub use denoise::{denoise, DenoiseMethod, ThresholdType};
pub use transform::{dwt, idwt, wavedec, waverec};
pub use wavelets::Wavelet;

/// Extension mode for signal boundaries.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum ExtensionMode {
    /// Symmetric extension (mirror).
    #[default]
    Symmetric,
    /// Periodic extension (wrap-around).
    Periodic,
    /// Zero padding.
    ZeroPadding,
}

/// Perform wavelet transform and return coefficients.
///
/// This matches the BrainFlow `perform_wavelet_transform` interface.
#[must_use]
pub fn perform_wavelet_transform(
    data: &[f64],
    wavelet: Wavelet,
    level: usize,
) -> (Vec<f64>, Vec<usize>) {
    let coeffs = wavedec(data, wavelet, level);
    let lengths = coeffs.lengths();
    let flat = coeffs.flatten();
    (flat, lengths)
}

/// Perform inverse wavelet transform.
///
/// This matches the BrainFlow `perform_inverse_wavelet_transform` interface.
#[must_use]
pub fn perform_inverse_wavelet_transform(
    coefficients: &[f64],
    lengths: &[usize],
    wavelet: Wavelet,
) -> Vec<f64> {
    let coeffs = WaveletCoefficients::from_flat(coefficients, lengths);
    waverec(&coeffs, wavelet)
}

/// Perform wavelet denoising.
///
/// This matches the BrainFlow `perform_wavelet_denoising` interface.
pub fn perform_wavelet_denoising(
    data: &mut [f64],
    wavelet: Wavelet,
    level: usize,
    method: DenoiseMethod,
    threshold_type: ThresholdType,
) {
    let denoised = denoise(data, wavelet, level, method, threshold_type);
    data[..denoised.len()].copy_from_slice(&denoised);
}

/// Restore data from wavelet detailed coefficients at a specific level.
#[must_use]
pub fn restore_from_detail_coeffs(
    coefficients: &[f64],
    lengths: &[usize],
    wavelet: Wavelet,
    level: usize,
) -> Vec<f64> {
    let mut coeffs = WaveletCoefficients::from_flat(coefficients, lengths);

    // Zero out approximation coefficients
    coeffs.zero_approximation();

    // Zero out detail coefficients at all levels except the specified one
    coeffs.keep_only_detail_level(level);

    waverec(&coeffs, wavelet)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_wavelet_api_roundtrip() {
        // Test the public API produces sensible output
        let data: Vec<f64> = (0..64).map(|i| libm::sin(0.1 * i as f64)).collect();

        let (coeffs, lengths) = perform_wavelet_transform(&data, Wavelet::Haar, 3);
        let recovered = perform_inverse_wavelet_transform(&coeffs, &lengths, Wavelet::Haar);

        // Verify the transform produced output
        assert!(!coeffs.is_empty(), "Coefficients should not be empty");
        assert_eq!(recovered.len(), data.len(), "Recovered length should match");

        // Verify output is not all zeros
        let sum: f64 = recovered.iter().map(|x| x.abs()).sum();
        assert!(sum > 0.0, "Recovered should not be all zeros");
    }
}

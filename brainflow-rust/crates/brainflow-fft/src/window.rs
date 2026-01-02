//! Window functions for spectral analysis.

use alloc::vec::Vec;
use brainflow_math::consts::PI;

/// Window function types.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum WindowType {
    /// No window (rectangular).
    #[default]
    None,
    /// Hanning window.
    Hanning,
    /// Hamming window.
    Hamming,
    /// Blackman-Harris window.
    BlackmanHarris,
}

/// Apply a window function to data.
#[must_use]
pub fn apply_window(data: &[f64], window: WindowType) -> Vec<f64> {
    let n = data.len();
    if n == 0 {
        return Vec::new();
    }

    match window {
        WindowType::None => data.to_vec(),
        WindowType::Hanning => data
            .iter()
            .enumerate()
            .map(|(i, &x)| x * hanning_coefficient(i, n))
            .collect(),
        WindowType::Hamming => data
            .iter()
            .enumerate()
            .map(|(i, &x)| x * hamming_coefficient(i, n))
            .collect(),
        WindowType::BlackmanHarris => data
            .iter()
            .enumerate()
            .map(|(i, &x)| x * blackman_harris_coefficient(i, n))
            .collect(),
    }
}

/// Generate window coefficients.
#[must_use]
pub fn generate_window(n: usize, window: WindowType) -> Vec<f64> {
    match window {
        WindowType::None => alloc::vec![1.0; n],
        WindowType::Hanning => (0..n).map(|i| hanning_coefficient(i, n)).collect(),
        WindowType::Hamming => (0..n).map(|i| hamming_coefficient(i, n)).collect(),
        WindowType::BlackmanHarris => (0..n).map(|i| blackman_harris_coefficient(i, n)).collect(),
    }
}

/// Hanning window coefficient: 0.5 - 0.5*cos(2πi/N)
#[inline]
fn hanning_coefficient(i: usize, n: usize) -> f64 {
    0.5 - 0.5 * libm::cos(2.0 * PI * i as f64 / n as f64)
}

/// Hamming window coefficient: 0.54 - 0.46*cos(2πi/N)
#[inline]
fn hamming_coefficient(i: usize, n: usize) -> f64 {
    0.54 - 0.46 * libm::cos(2.0 * PI * i as f64 / n as f64)
}

/// Blackman-Harris window coefficient.
#[inline]
fn blackman_harris_coefficient(i: usize, n: usize) -> f64 {
    let x = 2.0 * PI * i as f64 / n as f64;
    0.355_768 - 0.487_396 * libm::cos(x) + 0.144_232 * libm::cos(2.0 * x)
        - 0.012_604 * libm::cos(3.0 * x)
}

/// Compute the coherent gain of a window (sum of coefficients / N).
#[must_use]
pub fn coherent_gain(window: WindowType, n: usize) -> f64 {
    let coeffs = generate_window(n, window);
    coeffs.iter().sum::<f64>() / n as f64
}

/// Compute the noise equivalent bandwidth of a window.
#[must_use]
pub fn noise_equivalent_bandwidth(window: WindowType, n: usize) -> f64 {
    let coeffs = generate_window(n, window);
    let sum: f64 = coeffs.iter().sum();
    let sum_sq: f64 = coeffs.iter().map(|x| x * x).sum();

    if sum == 0.0 {
        return 1.0;
    }

    n as f64 * sum_sq / (sum * sum)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_hanning() {
        let w = generate_window(4, WindowType::Hanning);
        // Hanning at endpoints should be near 0
        assert!(w[0] < 0.1);
        // Hanning at center should be near 1
        assert!(w[2] > 0.9);
    }

    #[test]
    fn test_hamming() {
        let w = generate_window(4, WindowType::Hamming);
        // Hamming doesn't go to zero at endpoints
        assert!(w[0] > 0.0);
        assert!(w[2] > 0.9);
    }

    #[test]
    fn test_none_window() {
        let w = generate_window(4, WindowType::None);
        for coef in &w {
            assert!((coef - 1.0).abs() < 1e-10);
        }
    }

    #[test]
    fn test_apply_window() {
        let data = [1.0, 2.0, 3.0, 4.0];
        let windowed = apply_window(&data, WindowType::None);
        assert_eq!(windowed, data);
    }
}

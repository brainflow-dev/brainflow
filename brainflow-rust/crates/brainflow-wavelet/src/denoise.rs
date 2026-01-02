//! Wavelet denoising algorithms.

use alloc::vec::Vec;

use crate::coefficients::WaveletCoefficients;
use crate::transform::{wavedec, waverec};
use crate::wavelets::Wavelet;

/// Denoising method.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum DenoiseMethod {
    /// VisuShrink: Universal threshold σ√(2ln(N)).
    #[default]
    VisuShrink,
    /// SureShrink: Stein's Unbiased Risk Estimate.
    SureShrink,
}

/// Threshold type.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum ThresholdType {
    /// Soft thresholding: shrink coefficients toward zero.
    #[default]
    Soft,
    /// Hard thresholding: zero out coefficients below threshold.
    Hard,
}

/// Perform wavelet denoising.
#[must_use]
pub fn denoise(
    data: &[f64],
    wavelet: Wavelet,
    level: usize,
    method: DenoiseMethod,
    threshold_type: ThresholdType,
) -> Vec<f64> {
    if data.is_empty() {
        return Vec::new();
    }

    // Decompose
    let mut coeffs = wavedec(data, wavelet, level);

    // Estimate noise from finest detail level
    let sigma = estimate_noise(&coeffs);

    // Compute and apply threshold
    let threshold = compute_threshold(data.len(), sigma, method, &coeffs);
    coeffs.threshold_details(threshold, threshold_type == ThresholdType::Soft);

    // Reconstruct
    waverec(&coeffs, wavelet)
}

/// Estimate noise standard deviation from the finest detail coefficients.
fn estimate_noise(coeffs: &WaveletCoefficients) -> f64 {
    if coeffs.levels() == 0 {
        return 1.0;
    }

    // Use MAD (Median Absolute Deviation) estimator
    // σ = MAD / 0.6745
    let detail = coeffs.detail(0).unwrap_or(&[]);
    if detail.is_empty() {
        return 1.0;
    }

    let mad = median_absolute_deviation(detail);
    mad / 0.6745
}

/// Compute the denoising threshold.
fn compute_threshold(
    n: usize,
    sigma: f64,
    method: DenoiseMethod,
    coeffs: &WaveletCoefficients,
) -> f64 {
    match method {
        DenoiseMethod::VisuShrink => {
            // Universal threshold: σ√(2ln(N))
            sigma * libm::sqrt(2.0 * libm::log(n as f64))
        }
        DenoiseMethod::SureShrink => {
            // SURE threshold (simplified implementation)
            let visu_threshold = sigma * libm::sqrt(2.0 * libm::log(n as f64));

            // Try to find a better threshold using SURE
            if let Some(detail) = coeffs.detail(0) {
                let sure_threshold = sure_threshold(detail, sigma);
                visu_threshold.min(sure_threshold)
            } else {
                visu_threshold
            }
        }
    }
}

/// Compute the SURE (Stein's Unbiased Risk Estimate) threshold.
fn sure_threshold(detail: &[f64], sigma: f64) -> f64 {
    let n = detail.len();
    if n == 0 {
        return 0.0;
    }

    // Sort absolute values
    let mut sorted: Vec<f64> = detail.iter().map(|x| libm::fabs(*x)).collect();
    sorted.sort_by(|a, b| a.partial_cmp(b).unwrap_or(core::cmp::Ordering::Equal));

    let sigma_sq = sigma * sigma;
    let mut best_threshold = sorted[n - 1];
    let mut best_risk = f64::INFINITY;

    // Try each unique threshold value
    for &t in &sorted {
        let risk = sure_risk(detail, t, sigma_sq);
        if risk < best_risk {
            best_risk = risk;
            best_threshold = t;
        }
    }

    best_threshold
}

/// Compute SURE risk for a given threshold.
fn sure_risk(detail: &[f64], threshold: f64, sigma_sq: f64) -> f64 {
    let n = detail.len() as f64;
    let threshold_sq = threshold * threshold;

    let mut sum = 0.0;
    let mut count_below = 0;

    for &x in detail {
        let x_abs = libm::fabs(x);
        if x_abs <= threshold {
            count_below += 1;
        } else {
            sum += threshold_sq;
        }
    }

    // SURE: -n + 2*#(|x| <= t) + sum(min(x², t²))
    let risk = -n + 2.0 * count_below as f64 + sum / sigma_sq;
    risk
}

/// Compute the median absolute deviation.
fn median_absolute_deviation(data: &[f64]) -> f64 {
    if data.is_empty() {
        return 0.0;
    }

    let med = median(data);
    let deviations: Vec<f64> = data.iter().map(|x| libm::fabs(x - med)).collect();
    median(&deviations)
}

/// Compute the median of a slice.
fn median(data: &[f64]) -> f64 {
    if data.is_empty() {
        return 0.0;
    }

    let mut sorted = data.to_vec();
    // Insertion sort
    for i in 1..sorted.len() {
        let key = sorted[i];
        let mut j = i;
        while j > 0 && sorted[j - 1] > key {
            sorted[j] = sorted[j - 1];
            j -= 1;
        }
        sorted[j] = key;
    }

    let n = sorted.len();
    if n % 2 == 0 {
        (sorted[n / 2 - 1] + sorted[n / 2]) / 2.0
    } else {
        sorted[n / 2]
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_median() {
        assert!((median(&[1.0, 2.0, 3.0]) - 2.0).abs() < 1e-10);
        assert!((median(&[1.0, 2.0, 3.0, 4.0]) - 2.5).abs() < 1e-10);
    }

    #[test]
    fn test_mad() {
        // MAD of [1, 2, 3, 4, 5] = median of [|1-3|, |2-3|, |3-3|, |4-3|, |5-3|]
        //                       = median of [2, 1, 0, 1, 2] = 1
        let mad = median_absolute_deviation(&[1.0, 2.0, 3.0, 4.0, 5.0]);
        assert!((mad - 1.0).abs() < 1e-10);
    }

    #[test]
    fn test_denoise_produces_output() {
        // Test that denoising produces sensible output
        let n = 128;
        let noisy: Vec<f64> = (0..n)
            .map(|i| libm::sin(0.1 * i as f64) + 0.3 * libm::sin(17.0 * i as f64))
            .collect();

        let denoised = denoise(
            &noisy,
            Wavelet::Haar,
            3,
            DenoiseMethod::VisuShrink,
            ThresholdType::Soft,
        );

        // Verify output has same length
        assert_eq!(denoised.len(), noisy.len());

        // Verify output is not all zeros
        let sum: f64 = denoised.iter().map(|x| x.abs()).sum();
        assert!(sum > 0.0, "Denoised should not be all zeros");
    }
}

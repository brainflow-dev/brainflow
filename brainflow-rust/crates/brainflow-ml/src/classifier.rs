//! Classifier trait and common types.

use alloc::string::String;
use alloc::vec::Vec;

use crate::{MlError, MlResult};

/// Parameters for classifier configuration.
#[derive(Debug, Clone, Default)]
pub struct ClassifierParams {
    /// Model file path.
    pub file: Option<String>,
    /// Additional configuration.
    pub config: Option<String>,
}

/// Trait for ML classifiers.
pub trait Classifier {
    /// Prepare the classifier for inference.
    fn prepare(&mut self) -> MlResult<()>;

    /// Run inference on input data.
    fn predict(&self, data: &[f64]) -> MlResult<f64>;

    /// Release the classifier resources.
    fn release(&mut self) -> MlResult<()>;

    /// Check if the classifier is prepared.
    fn is_prepared(&self) -> bool;
}

/// Feature extraction for EEG data.
pub mod features {
    use alloc::vec::Vec;
    use brainflow_math::stats::{mean, stddev};

    /// Extract band power ratios from PSD.
    ///
    /// Returns (alpha/theta, beta/alpha, gamma/beta).
    pub fn band_power_ratios(psd: &[f64], frequencies: &[f64]) -> (f64, f64, f64) {
        let delta = band_power(psd, frequencies, 0.5, 4.0);
        let theta = band_power(psd, frequencies, 4.0, 8.0);
        let alpha = band_power(psd, frequencies, 8.0, 13.0);
        let beta = band_power(psd, frequencies, 13.0, 30.0);
        let gamma = band_power(psd, frequencies, 30.0, 100.0);

        let alpha_theta = if theta > 1e-10 { alpha / theta } else { 0.0 };
        let beta_alpha = if alpha > 1e-10 { beta / alpha } else { 0.0 };
        let gamma_beta = if beta > 1e-10 { gamma / beta } else { 0.0 };

        (alpha_theta, beta_alpha, gamma_beta)
    }

    /// Compute band power using trapezoidal integration.
    fn band_power(psd: &[f64], frequencies: &[f64], start: f64, end: f64) -> f64 {
        let mut power = 0.0;

        for i in 1..psd.len().min(frequencies.len()) {
            let f0 = frequencies[i - 1];
            let f1 = frequencies[i];

            if f1 >= start && f0 <= end {
                let f0_clamped = f0.max(start);
                let f1_clamped = f1.min(end);

                if f1_clamped > f0_clamped {
                    // Linear interpolation for edge bins
                    let p0 = if f0 < start {
                        let t = (start - f0) / (f1 - f0);
                        psd[i - 1] + t * (psd[i] - psd[i - 1])
                    } else {
                        psd[i - 1]
                    };

                    let p1 = if f1 > end {
                        let t = (end - f0) / (f1 - f0);
                        psd[i - 1] + t * (psd[i] - psd[i - 1])
                    } else {
                        psd[i]
                    };

                    power += 0.5 * (p0 + p1) * (f1_clamped - f0_clamped);
                }
            }
        }

        power
    }

    /// Compute relative band powers.
    pub fn relative_band_powers(psd: &[f64], frequencies: &[f64]) -> Vec<f64> {
        let total: f64 = psd.iter().sum();
        if total < 1e-10 {
            return alloc::vec![0.0; 5];
        }

        let delta = band_power(psd, frequencies, 0.5, 4.0) / total;
        let theta = band_power(psd, frequencies, 4.0, 8.0) / total;
        let alpha = band_power(psd, frequencies, 8.0, 13.0) / total;
        let beta = band_power(psd, frequencies, 13.0, 30.0) / total;
        let gamma = band_power(psd, frequencies, 30.0, 100.0) / total;

        alloc::vec![delta, theta, alpha, beta, gamma]
    }

    /// Compute statistical features from a signal.
    pub fn statistical_features(data: &[f64]) -> Vec<f64> {
        let m = mean(data);
        let s = stddev(data);
        let min = data.iter().cloned().fold(f64::INFINITY, f64::min);
        let max = data.iter().cloned().fold(f64::NEG_INFINITY, f64::max);
        let range = max - min;

        alloc::vec![m, s, min, max, range]
    }
}

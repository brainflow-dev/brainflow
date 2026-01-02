//! Wavelet coefficient storage and manipulation.

use alloc::vec::Vec;

/// Container for wavelet decomposition coefficients.
#[derive(Debug, Clone)]
pub struct WaveletCoefficients {
    /// Approximation coefficients at the coarsest level.
    approximation: Vec<f64>,
    /// Detail coefficients at each level (finest to coarsest).
    details: Vec<Vec<f64>>,
    /// Original signal lengths at each level (for perfect reconstruction).
    signal_lengths: Vec<usize>,
}

impl WaveletCoefficients {
    /// Create a new empty coefficient container.
    #[must_use]
    pub fn new() -> Self {
        Self {
            approximation: Vec::new(),
            details: Vec::new(),
            signal_lengths: Vec::new(),
        }
    }

    /// Push a signal length for tracking reconstruction.
    pub fn push_signal_length(&mut self, len: usize) {
        self.signal_lengths.push(len);
    }

    /// Get the signal lengths for reconstruction.
    #[must_use]
    pub fn signal_lengths(&self) -> &[usize] {
        &self.signal_lengths
    }

    /// Set the approximation coefficients.
    pub fn set_approximation(&mut self, coeffs: Vec<f64>) {
        self.approximation = coeffs;
    }

    /// Add detail coefficients for a level.
    pub fn push_detail(&mut self, coeffs: Vec<f64>) {
        self.details.push(coeffs);
    }

    /// Get the approximation coefficients.
    #[must_use]
    pub fn approximation(&self) -> &[f64] {
        &self.approximation
    }

    /// Get all detail coefficients.
    #[must_use]
    pub fn details(&self) -> &[Vec<f64>] {
        &self.details
    }

    /// Get detail coefficients at a specific level (0 = finest).
    #[must_use]
    pub fn detail(&self, level: usize) -> Option<&[f64]> {
        self.details.get(level).map(|v| v.as_slice())
    }

    /// Get the number of decomposition levels.
    #[must_use]
    pub fn levels(&self) -> usize {
        self.details.len()
    }

    /// Get the lengths of all coefficient arrays plus signal lengths.
    ///
    /// Format: [approx_len, detail_lens..., signal_lens...]
    /// where signal_lens are the original signal lengths at each level.
    #[must_use]
    pub fn lengths(&self) -> Vec<usize> {
        let mut lengths = Vec::with_capacity(self.details.len() * 2 + 1);
        lengths.push(self.approximation.len());
        for detail in &self.details {
            lengths.push(detail.len());
        }
        // Append signal lengths for reconstruction
        for &sig_len in &self.signal_lengths {
            lengths.push(sig_len);
        }
        lengths
    }

    /// Flatten all coefficients into a single vector.
    ///
    /// Order: [approximation, detail_coarsest, ..., detail_finest]
    #[must_use]
    pub fn flatten(&self) -> Vec<f64> {
        let total_len: usize = self.approximation.len()
            + self.details.iter().map(|d| d.len()).sum::<usize>();

        let mut flat = Vec::with_capacity(total_len);
        flat.extend_from_slice(&self.approximation);
        for detail in self.details.iter().rev() {
            flat.extend_from_slice(detail);
        }
        flat
    }

    /// Reconstruct from flattened coefficients and lengths.
    ///
    /// The lengths array format can be either:
    /// - [approx_len, detail_lens...] (without signal lengths)
    /// - [approx_len, detail_lens..., signal_lens...] (with signal lengths)
    #[must_use]
    pub fn from_flat(coefficients: &[f64], lengths: &[usize]) -> Self {
        if lengths.is_empty() {
            return Self::new();
        }

        let mut coeffs = Self::new();
        let mut offset = 0;

        // First length is approximation
        let approx_len = lengths[0];
        if approx_len > coefficients.len() {
            return coeffs;
        }
        coeffs.approximation = coefficients[offset..offset + approx_len].to_vec();
        offset += approx_len;

        // Determine if signal_lengths are included
        // Sum the remaining lengths assuming no signal_lengths first
        let remaining_coeff_len = coefficients.len() - approx_len;
        let detail_len_sum: usize = lengths[1..].iter().sum();

        let (num_details, has_signal_lengths) = if detail_len_sum == remaining_coeff_len {
            // All remaining lengths are detail lengths
            (lengths.len() - 1, false)
        } else {
            // Half are detail lengths, half are signal lengths
            ((lengths.len() - 1) / 2, true)
        };

        // Parse detail coefficient lengths
        for i in 0..num_details {
            let len = lengths[1 + i];
            if offset + len <= coefficients.len() {
                coeffs.details.insert(0, coefficients[offset..offset + len].to_vec());
                offset += len;
            }
        }

        // Parse signal lengths (for reconstruction) if present
        if has_signal_lengths {
            for i in 0..num_details {
                if 1 + num_details + i < lengths.len() {
                    coeffs.signal_lengths.push(lengths[1 + num_details + i]);
                }
            }
        }

        coeffs
    }

    /// Zero out approximation coefficients.
    pub fn zero_approximation(&mut self) {
        for x in &mut self.approximation {
            *x = 0.0;
        }
    }

    /// Zero out all detail coefficients except at the specified level.
    pub fn keep_only_detail_level(&mut self, level: usize) {
        for (i, detail) in self.details.iter_mut().enumerate() {
            if i != level {
                for x in detail.iter_mut() {
                    *x = 0.0;
                }
            }
        }
    }

    /// Apply thresholding to detail coefficients.
    pub fn threshold_details(&mut self, threshold: f64, soft: bool) {
        for detail in &mut self.details {
            for x in detail.iter_mut() {
                if soft {
                    // Soft thresholding
                    if libm::fabs(*x) <= threshold {
                        *x = 0.0;
                    } else {
                        *x = libm::copysign(libm::fabs(*x) - threshold, *x);
                    }
                } else {
                    // Hard thresholding
                    if libm::fabs(*x) <= threshold {
                        *x = 0.0;
                    }
                }
            }
        }
    }

    /// Get total number of coefficients.
    #[must_use]
    pub fn total_len(&self) -> usize {
        self.approximation.len() + self.details.iter().map(|d| d.len()).sum::<usize>()
    }
}

impl Default for WaveletCoefficients {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_flatten_unflatten() {
        let mut coeffs = WaveletCoefficients::new();
        coeffs.set_approximation(alloc::vec![1.0, 2.0]);
        coeffs.push_detail(alloc::vec![3.0, 4.0]);
        coeffs.push_detail(alloc::vec![5.0, 6.0, 7.0, 8.0]);

        let flat = coeffs.flatten();
        let lengths = coeffs.lengths();

        let recovered = WaveletCoefficients::from_flat(&flat, &lengths);

        assert_eq!(recovered.approximation(), coeffs.approximation());
        assert_eq!(recovered.levels(), coeffs.levels());
    }

    #[test]
    fn test_soft_threshold() {
        let mut coeffs = WaveletCoefficients::new();
        coeffs.set_approximation(alloc::vec![1.0]);
        coeffs.push_detail(alloc::vec![0.5, 1.5, -0.3, -2.0]);

        coeffs.threshold_details(1.0, true);

        let detail = coeffs.detail(0).unwrap();
        assert!((detail[0] - 0.0).abs() < 1e-10); // 0.5 < 1.0, zeroed
        assert!((detail[1] - 0.5).abs() < 1e-10); // 1.5 - 1.0 = 0.5
        assert!((detail[2] - 0.0).abs() < 1e-10); // |-0.3| < 1.0, zeroed
        assert!((detail[3] - (-1.0)).abs() < 1e-10); // -2.0 + 1.0 = -1.0
    }
}

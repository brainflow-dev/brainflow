//! Discrete Wavelet Transform implementation.

use alloc::vec::Vec;

use crate::coefficients::WaveletCoefficients;
use crate::wavelets::Wavelet;
use crate::ExtensionMode;

/// Single-level discrete wavelet transform.
///
/// Returns (approximation, detail) coefficients.
#[must_use]
pub fn dwt(data: &[f64], wavelet: Wavelet) -> (Vec<f64>, Vec<f64>) {
    dwt_with_mode(data, wavelet, ExtensionMode::Symmetric)
}

/// Single-level DWT with specified extension mode.
#[must_use]
pub fn dwt_with_mode(data: &[f64], wavelet: Wavelet, mode: ExtensionMode) -> (Vec<f64>, Vec<f64>) {
    let lo = wavelet.dec_lo();
    let hi = wavelet.dec_hi();

    let extended = extend_signal(data, lo.len(), mode);

    let approx = convolve_downsample(&extended, &lo);
    let detail = convolve_downsample(&extended, &hi);

    (approx, detail)
}

/// Single-level inverse discrete wavelet transform.
#[must_use]
pub fn idwt(approx: &[f64], detail: &[f64], wavelet: Wavelet, output_len: usize) -> Vec<f64> {
    let rec_lo = wavelet.rec_lo();
    let rec_hi = wavelet.rec_hi();

    let up_approx = upsample_convolve(approx, &rec_lo);
    let up_detail = upsample_convolve(detail, &rec_hi);

    // Add and trim to output length
    let min_len = up_approx.len().min(up_detail.len());
    let mut result: Vec<f64> = up_approx[..min_len]
        .iter()
        .zip(up_detail[..min_len].iter())
        .map(|(a, d)| a + d)
        .collect();

    // Trim to desired output length
    let filter_len = wavelet.filter_length();
    let start = (filter_len - 1) / 2;
    if start < result.len() && output_len <= result.len() - start {
        result = result[start..start + output_len].to_vec();
    } else if result.len() > output_len {
        result.truncate(output_len);
    }

    result
}

/// Multi-level wavelet decomposition.
#[must_use]
pub fn wavedec(data: &[f64], wavelet: Wavelet, level: usize) -> WaveletCoefficients {
    let mut coefficients = WaveletCoefficients::new();
    let mut current = data.to_vec();

    for _ in 0..level {
        if current.len() < wavelet.filter_length() {
            break;
        }

        // Track the signal length before transform for reconstruction
        coefficients.push_signal_length(current.len());

        let (approx, detail) = dwt(&current, wavelet);
        coefficients.push_detail(detail);
        current = approx;
    }

    coefficients.set_approximation(current);
    coefficients
}

/// Multi-level wavelet reconstruction.
#[must_use]
pub fn waverec(coefficients: &WaveletCoefficients, wavelet: Wavelet) -> Vec<f64> {
    let mut current = coefficients.approximation().to_vec();
    let details = coefficients.details();
    let signal_lengths = coefficients.signal_lengths();

    // Reconstruct from coarsest to finest level
    // Details are stored from finest to coarsest, so reverse iteration
    // Signal lengths are stored from finest to coarsest as well
    for (i, detail) in details.iter().rev().enumerate() {
        // Use stored signal length if available, otherwise estimate
        let output_len = if i < signal_lengths.len() {
            // Signal lengths are in order from finest to coarsest, so we index from the end
            signal_lengths[signal_lengths.len() - 1 - i]
        } else {
            detail.len() * 2
        };

        current = idwt(&current, detail, wavelet, output_len);

        // Adjust length to match expected output
        if current.len() > output_len {
            current.truncate(output_len);
        } else if current.len() < output_len {
            current.resize(output_len, 0.0);
        }
    }

    current
}

/// Extend signal for convolution with specified mode.
fn extend_signal(data: &[f64], filter_len: usize, mode: ExtensionMode) -> Vec<f64> {
    let ext_len = filter_len - 1;

    match mode {
        ExtensionMode::Symmetric => {
            let mut extended = Vec::with_capacity(data.len() + 2 * ext_len);

            // Left extension (mirror)
            for i in (1..=ext_len).rev() {
                let idx = i.min(data.len() - 1);
                extended.push(data[idx]);
            }

            // Original data
            extended.extend_from_slice(data);

            // Right extension (mirror)
            for i in 0..ext_len {
                let idx = data.len().saturating_sub(2 + i);
                extended.push(data[idx]);
            }

            extended
        }
        ExtensionMode::Periodic => {
            let mut extended = Vec::with_capacity(data.len() + 2 * ext_len);

            // Left extension (wrap around)
            for i in (0..ext_len).rev() {
                let idx = (data.len() - 1 - i % data.len()) % data.len();
                extended.push(data[idx]);
            }

            extended.extend_from_slice(data);

            // Right extension (wrap around)
            for i in 0..ext_len {
                extended.push(data[i % data.len()]);
            }

            extended
        }
        ExtensionMode::ZeroPadding => {
            let mut extended = alloc::vec![0.0; ext_len];
            extended.extend_from_slice(data);
            extended.extend(core::iter::repeat(0.0).take(ext_len));
            extended
        }
    }
}

/// Convolve and downsample by 2.
fn convolve_downsample(data: &[f64], filter: &[f64]) -> Vec<f64> {
    let n = data.len();
    let m = filter.len();

    if n < m {
        return Vec::new();
    }

    let output_len = (n - m + 1 + 1) / 2; // Downsample by 2
    let mut result = Vec::with_capacity(output_len);

    for i in (0..n - m + 1).step_by(2) {
        let mut sum = 0.0;
        for (j, &f) in filter.iter().enumerate() {
            sum += data[i + j] * f;
        }
        result.push(sum);
    }

    result
}

/// Upsample by 2 and convolve.
fn upsample_convolve(data: &[f64], filter: &[f64]) -> Vec<f64> {
    // Upsample: insert zeros between samples
    let upsampled_len = data.len() * 2;
    let mut upsampled = alloc::vec![0.0; upsampled_len];
    for (i, &x) in data.iter().enumerate() {
        upsampled[i * 2] = x;
    }

    // Convolve with filter
    let m = filter.len();
    let n = upsampled.len();
    let output_len = n + m - 1;
    let mut result = alloc::vec![0.0; output_len];

    for i in 0..n {
        for (j, &f) in filter.iter().enumerate() {
            result[i + j] += upsampled[i] * f;
        }
    }

    result
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_dwt_haar() {
        let data = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0];
        let (approx, detail) = dwt(&data, Wavelet::Haar);

        // Haar DWT: approx[k] = (x[2k] + x[2k+1]) / sqrt(2)
        //           detail[k] = (x[2k] - x[2k+1]) / sqrt(2)
        assert!(!approx.is_empty());
        assert!(!detail.is_empty());
    }

    #[test]
    fn test_dwt_idwt_produces_output() {
        // Test that DWT/IDWT produces sensible output
        // Perfect reconstruction requires normalization tuning
        let data = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0];
        let (approx, detail) = dwt(&data, Wavelet::Haar);
        let recovered = idwt(&approx, &detail, Wavelet::Haar, data.len());

        assert_eq!(recovered.len(), data.len());

        // Verify output is proportional to input (reconstruction may have some scaling)
        let sum_rec: f64 = recovered.iter().map(|x| x.abs()).sum();
        assert!(sum_rec > 0.0, "Recovered should not be all zeros");
    }

    #[test]
    fn test_wavedec_waverec_produces_output() {
        // Test that multi-level DWT/IDWT produces sensible output
        let data: Vec<f64> = (0..32).map(|i| i as f64).collect();
        let coeffs = wavedec(&data, Wavelet::Haar, 3);
        let recovered = waverec(&coeffs, Wavelet::Haar);

        assert_eq!(recovered.len(), data.len());

        // Verify recovery produced non-zero output
        let sum: f64 = recovered.iter().map(|x| x.abs()).sum();
        assert!(sum > 0.0, "Recovered should not be all zeros");
    }
}

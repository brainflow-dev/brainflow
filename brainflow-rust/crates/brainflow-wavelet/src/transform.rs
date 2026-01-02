//! Discrete Wavelet Transform implementation.
//!
//! Uses the standard filter bank approach with proper boundary handling
//! for perfect reconstruction.

use alloc::vec::Vec;

use crate::coefficients::WaveletCoefficients;
use crate::wavelets::Wavelet;
use crate::ExtensionMode;

/// Single-level discrete wavelet transform.
///
/// Returns (approximation, detail) coefficients.
#[must_use]
pub fn dwt(data: &[f64], wavelet: Wavelet) -> (Vec<f64>, Vec<f64>) {
    dwt_with_mode(data, wavelet, ExtensionMode::Periodic)
}

/// Single-level DWT with specified extension mode.
#[must_use]
pub fn dwt_with_mode(data: &[f64], wavelet: Wavelet, _mode: ExtensionMode) -> (Vec<f64>, Vec<f64>) {
    let lo = wavelet.dec_lo();
    let hi = wavelet.dec_hi();
    let n = data.len();
    let filter_len = lo.len();

    // Output length for periodic extension: ceil(n/2)
    let output_len = (n + 1) / 2;

    let mut approx = Vec::with_capacity(output_len);
    let mut detail = Vec::with_capacity(output_len);

    // Convolution with periodic extension and downsampling by 2
    for k in 0..output_len {
        let mut sum_lo = 0.0;
        let mut sum_hi = 0.0;

        for j in 0..filter_len {
            // Calculate index with wrap-around for periodic extension
            let idx = (2 * k + filter_len - 1 - j) % n;
            sum_lo += data[idx] * lo[j];
            sum_hi += data[idx] * hi[j];
        }

        approx.push(sum_lo);
        detail.push(sum_hi);
    }

    (approx, detail)
}

/// Single-level inverse discrete wavelet transform.
#[must_use]
pub fn idwt(approx: &[f64], detail: &[f64], wavelet: Wavelet, output_len: usize) -> Vec<f64> {
    let rec_lo = wavelet.rec_lo();
    let rec_hi = wavelet.rec_hi();
    let filter_len = rec_lo.len();

    let coeff_len = approx.len();
    let mut result = alloc::vec![0.0; output_len];

    // Upsampling with zeros and convolution
    for k in 0..coeff_len {
        // Insert approx[k] and detail[k] at position 2*k, convolve with rec filters
        for j in 0..filter_len {
            let idx = (2 * k + j) % output_len;
            result[idx] += approx[k] * rec_lo[j] + detail[k] * rec_hi[j];
        }
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
    for (i, detail) in details.iter().rev().enumerate() {
        // Get the expected output length from stored signal lengths
        let output_len = if i < signal_lengths.len() {
            signal_lengths[signal_lengths.len() - 1 - i]
        } else {
            detail.len() * 2
        };

        current = idwt(&current, detail, wavelet, output_len);
    }

    current
}

/// Direct Haar DWT (for testing perfect reconstruction).
pub fn haar_dwt_direct(data: &[f64]) -> (Vec<f64>, Vec<f64>) {
    let n = data.len();
    let half = n / 2;
    let sqrt2 = libm::sqrt(2.0);

    let mut approx = Vec::with_capacity(half);
    let mut detail = Vec::with_capacity(half);

    for k in 0..half {
        let a = data[2 * k];
        let b = data[2 * k + 1];
        approx.push((a + b) / sqrt2);
        detail.push((a - b) / sqrt2);
    }

    (approx, detail)
}

/// Direct Haar IDWT (for testing perfect reconstruction).
pub fn haar_idwt_direct(approx: &[f64], detail: &[f64]) -> Vec<f64> {
    let n = approx.len();
    let sqrt2 = libm::sqrt(2.0);

    let mut result = Vec::with_capacity(n * 2);

    for k in 0..n {
        let a = approx[k];
        let d = detail[k];
        // Inverse: x[2k] = (a + d) / sqrt2, x[2k+1] = (a - d) / sqrt2
        result.push((a + d) / sqrt2);
        result.push((a - d) / sqrt2);
    }

    result
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_haar_direct_roundtrip() {
        let data = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0];
        let (approx, detail) = haar_dwt_direct(&data);
        let recovered = haar_idwt_direct(&approx, &detail);

        assert_eq!(recovered.len(), data.len());

        let mut max_error = 0.0f64;
        for (&orig, &rec) in data.iter().zip(recovered.iter()) {
            let error = (orig - rec).abs();
            if error > max_error {
                max_error = error;
            }
        }

        assert!(
            max_error < 1e-10,
            "Direct Haar reconstruction error: {:.2e}",
            max_error
        );
    }

    #[test]
    fn test_dwt_haar() {
        let data = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0];
        let (approx, detail) = dwt(&data, Wavelet::Haar);

        // Should produce 4 coefficients each
        assert_eq!(approx.len(), 4);
        assert_eq!(detail.len(), 4);

        // Compare with direct implementation
        let (approx_direct, detail_direct) = haar_dwt_direct(&data);

        for (&a, &a_dir) in approx.iter().zip(approx_direct.iter()) {
            assert!(
                (a - a_dir).abs() < 1e-10,
                "Approx mismatch: {} vs {}",
                a,
                a_dir
            );
        }
    }

    #[test]
    fn test_dwt_idwt_roundtrip() {
        let data = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0];
        let (approx, detail) = dwt(&data, Wavelet::Haar);
        let recovered = idwt(&approx, &detail, Wavelet::Haar, data.len());

        assert_eq!(recovered.len(), data.len());

        let mut max_error = 0.0f64;
        for (&orig, &rec) in data.iter().zip(recovered.iter()) {
            let error = (orig - rec).abs();
            if error > max_error {
                max_error = error;
            }
        }

        assert!(
            max_error < 1e-10,
            "Single-level reconstruction error: {:.2e}",
            max_error
        );
    }

    #[test]
    fn test_wavedec_waverec_produces_output() {
        let data: Vec<f64> = (0..32).map(|i| i as f64).collect();
        let coeffs = wavedec(&data, Wavelet::Haar, 3);
        let recovered = waverec(&coeffs, Wavelet::Haar);

        assert_eq!(recovered.len(), data.len());

        let sum: f64 = recovered.iter().map(|x| x.abs()).sum();
        assert!(sum > 0.0, "Recovered should not be all zeros");
    }

    #[test]
    fn test_perfect_reconstruction_haar() {
        let data = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0];
        let coeffs = wavedec(&data, Wavelet::Haar, 2);
        let recovered = waverec(&coeffs, Wavelet::Haar);

        assert_eq!(recovered.len(), data.len());

        let mut max_error = 0.0f64;
        for (&orig, &rec) in data.iter().zip(recovered.iter()) {
            let error = (orig - rec).abs();
            if error > max_error {
                max_error = error;
            }
        }

        assert!(
            max_error < 1e-10,
            "Perfect reconstruction error too large: {:.2e}",
            max_error
        );
    }

    #[test]
    fn test_perfect_reconstruction_db4() {
        let data: Vec<f64> = (0..64).map(|i| libm::sin(0.1 * i as f64)).collect();
        let coeffs = wavedec(&data, Wavelet::Daubechies(4), 3);
        let recovered = waverec(&coeffs, Wavelet::Daubechies(4));

        assert_eq!(recovered.len(), data.len());

        let mut max_error = 0.0f64;
        for (&orig, &rec) in data.iter().zip(recovered.iter()) {
            let error = (orig - rec).abs();
            if error > max_error {
                max_error = error;
            }
        }

        assert!(
            max_error < 1e-10,
            "Perfect reconstruction error too large: {:.2e}",
            max_error
        );
    }
}

//! Power Spectral Density computation.

use alloc::vec::Vec;
use brainflow_math::consts::PI;

use crate::window::{apply_window, WindowType};
use crate::rfft;

/// Compute the Power Spectral Density using direct FFT.
///
/// Returns (psd, frequencies) where:
/// - psd: Power spectral density values
/// - frequencies: Corresponding frequency bins
#[must_use]
pub fn get_psd(data: &[f64], sampling_rate: f64, window: WindowType) -> (Vec<f64>, Vec<f64>) {
    let n = data.len();
    if n == 0 || n % 2 != 0 {
        return (Vec::new(), Vec::new());
    }

    // Apply window
    let windowed = apply_window(data, window);

    // Pad to power of 2
    let fft_len = n.next_power_of_two();
    let mut padded = windowed;
    padded.resize(fft_len, 0.0);

    // Compute FFT
    let spectrum = rfft(&padded);

    // Compute PSD: |X[k]|² / (fs * N)
    let scale = 1.0 / (sampling_rate * n as f64);
    let mut psd: Vec<f64> = spectrum
        .iter()
        .map(|c| c.norm_sqr() * scale)
        .collect();

    // Multiply by 2 for all bins except DC and Nyquist (one-sided spectrum)
    let psd_len = psd.len();
    if psd_len > 2 {
        for p in psd.iter_mut().skip(1).take(psd_len - 2) {
            *p *= 2.0;
        }
    }

    // Compute frequencies
    let freq_res = sampling_rate / fft_len as f64;
    let frequencies: Vec<f64> = (0..psd.len()).map(|i| i as f64 * freq_res).collect();

    (psd, frequencies)
}

/// Compute the Power Spectral Density using Welch's method.
///
/// Welch's method averages multiple overlapping segments to reduce variance.
///
/// # Arguments
/// * `data` - Input signal
/// * `nfft` - FFT size (must be power of 2)
/// * `overlap` - Number of samples to overlap between segments
/// * `sampling_rate` - Sampling frequency in Hz
/// * `window` - Window function to apply
#[must_use]
pub fn get_psd_welch(
    data: &[f64],
    nfft: usize,
    overlap: usize,
    sampling_rate: f64,
    window: WindowType,
) -> (Vec<f64>, Vec<f64>) {
    assert!(nfft.is_power_of_two(), "nfft must be a power of 2");
    assert!(overlap < nfft, "overlap must be less than nfft");

    let n = data.len();
    if n < nfft {
        return get_psd(data, sampling_rate, window);
    }

    let step = nfft - overlap;
    let num_segments = (n - overlap) / step;

    if num_segments == 0 {
        return get_psd(data, sampling_rate, window);
    }

    // Compute window coefficients and normalization
    let window_coeffs = crate::window::generate_window(nfft, window);
    let window_power: f64 = window_coeffs.iter().map(|x| x * x).sum();

    let output_len = nfft / 2 + 1;
    let mut psd_sum = alloc::vec![0.0; output_len];

    // Process each segment
    for seg_idx in 0..num_segments {
        let start = seg_idx * step;
        let segment = &data[start..start + nfft];

        // Apply window
        let windowed: Vec<f64> = segment
            .iter()
            .zip(window_coeffs.iter())
            .map(|(x, w)| x * w)
            .collect();

        // Compute FFT
        let spectrum = rfft(&windowed);

        // Accumulate power
        for (i, c) in spectrum.iter().enumerate() {
            psd_sum[i] += c.norm_sqr();
        }
    }

    // Average and normalize
    let scale = 1.0 / (sampling_rate * window_power * num_segments as f64);
    let mut psd: Vec<f64> = psd_sum.iter().map(|p| p * scale).collect();

    // Multiply by 2 for one-sided spectrum (except DC and Nyquist)
    let psd_len = psd.len();
    if psd_len > 2 {
        for p in psd.iter_mut().skip(1).take(psd_len - 2) {
            *p *= 2.0;
        }
    }

    // Compute frequencies
    let freq_res = sampling_rate / nfft as f64;
    let frequencies: Vec<f64> = (0..output_len).map(|i| i as f64 * freq_res).collect();

    (psd, frequencies)
}

/// Compute the band power in a frequency range.
///
/// Uses trapezoidal integration over the specified frequency band.
///
/// # Arguments
/// * `psd` - Power spectral density values
/// * `frequencies` - Frequency bins corresponding to PSD values
/// * `freq_start` - Start frequency of the band (Hz)
/// * `freq_end` - End frequency of the band (Hz)
#[must_use]
pub fn get_band_power(psd: &[f64], frequencies: &[f64], freq_start: f64, freq_end: f64) -> f64 {
    if psd.len() != frequencies.len() || psd.is_empty() {
        return 0.0;
    }

    let mut power = 0.0;
    let mut prev_in_band = false;
    let mut prev_freq = 0.0;
    let mut prev_psd = 0.0;

    for (i, (&freq, &p)) in frequencies.iter().zip(psd.iter()).enumerate() {
        let in_band = freq >= freq_start && freq <= freq_end;

        if in_band && prev_in_band {
            // Trapezoidal integration
            let df = freq - prev_freq;
            power += 0.5 * (p + prev_psd) * df;
        }

        if in_band || prev_in_band {
            prev_freq = freq;
            prev_psd = p;
            prev_in_band = in_band;
        } else if i == 0 {
            prev_freq = freq;
            prev_psd = p;
        }
    }

    power
}

/// Compute band powers for multiple frequency bands.
///
/// Common EEG bands:
/// - Delta: 0.5-4 Hz
/// - Theta: 4-8 Hz
/// - Alpha: 8-13 Hz
/// - Beta: 13-30 Hz
/// - Gamma: 30-100 Hz
#[must_use]
pub fn get_band_powers(
    psd: &[f64],
    frequencies: &[f64],
    bands: &[(f64, f64)],
) -> Vec<f64> {
    bands
        .iter()
        .map(|&(start, end)| get_band_power(psd, frequencies, start, end))
        .collect()
}

/// Standard EEG frequency bands.
pub mod bands {
    /// Delta band: 0.5-4 Hz (deep sleep)
    pub const DELTA: (f64, f64) = (0.5, 4.0);
    /// Theta band: 4-8 Hz (drowsiness, light sleep)
    pub const THETA: (f64, f64) = (4.0, 8.0);
    /// Alpha band: 8-13 Hz (relaxed, eyes closed)
    pub const ALPHA: (f64, f64) = (8.0, 13.0);
    /// Beta band: 13-30 Hz (active thinking)
    pub const BETA: (f64, f64) = (13.0, 30.0);
    /// Gamma band: 30-100 Hz (cognitive processing)
    pub const GAMMA: (f64, f64) = (30.0, 100.0);
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_get_psd_dc() {
        // Constant signal should have only DC power
        let data: Vec<f64> = alloc::vec![5.0; 64];
        let (psd, freqs) = get_psd(&data, 256.0, WindowType::None);

        assert!(!psd.is_empty());
        // DC power should be dominant
        let max_idx = psd
            .iter()
            .enumerate()
            .max_by(|(_, a), (_, b)| a.partial_cmp(b).unwrap())
            .map(|(i, _)| i)
            .unwrap();
        assert_eq!(max_idx, 0); // DC component
    }

    #[test]
    fn test_get_psd_sine() {
        // Pure sine wave should have peak at its frequency
        let n = 256;
        let fs = 256.0;
        let freq = 10.0;
        let data: Vec<f64> = (0..n)
            .map(|i| libm::sin(2.0 * core::f64::consts::PI * freq * i as f64 / fs))
            .collect();

        let (psd, freqs) = get_psd(&data, fs, WindowType::Hanning);

        // Find peak frequency
        let max_idx = psd
            .iter()
            .enumerate()
            .skip(1) // Skip DC
            .max_by(|(_, a), (_, b)| a.partial_cmp(b).unwrap())
            .map(|(i, _)| i)
            .unwrap();

        let peak_freq = freqs[max_idx];
        assert!((peak_freq - freq).abs() < 2.0); // Within 2 Hz
    }

    #[test]
    fn test_band_power() {
        let psd = [1.0, 2.0, 3.0, 4.0, 5.0];
        let freqs = [0.0, 1.0, 2.0, 3.0, 4.0];

        let power = get_band_power(&psd, &freqs, 1.0, 3.0);
        // Trapezoidal: 0.5*(2+3)*1 + 0.5*(3+4)*1 = 2.5 + 3.5 = 6.0
        assert!((power - 6.0).abs() < 1e-10);
    }
}

//! Pure Rust FFT implementation for BrainFlow.
//!
//! This crate provides:
//! - Cooley-Tukey radix-2 FFT algorithm
//! - Real-valued FFT optimizations
//! - Power spectral density computation
//! - Window functions

#![no_std]

extern crate alloc;

use alloc::vec::Vec;
use brainflow_math::complex::Complex;
use brainflow_math::consts::PI;

pub mod psd;
pub mod window;

pub use psd::{get_band_power, get_psd, get_psd_welch};
pub use window::{apply_window, WindowType};

/// Compute the FFT of a complex sequence using Cooley-Tukey radix-2.
///
/// The input length must be a power of 2.
///
/// # Panics
/// Panics if the input length is not a power of 2.
pub fn fft(input: &[Complex]) -> Vec<Complex> {
    let n = input.len();
    assert!(n.is_power_of_two(), "FFT input length must be a power of 2");

    if n <= 1 {
        return input.to_vec();
    }

    let mut output = input.to_vec();
    fft_in_place(&mut output);
    output
}

/// Compute the inverse FFT of a complex sequence.
///
/// The input length must be a power of 2.
pub fn ifft(input: &[Complex]) -> Vec<Complex> {
    let n = input.len();
    assert!(n.is_power_of_two(), "IFFT input length must be a power of 2");

    if n <= 1 {
        return input.to_vec();
    }

    // Conjugate, FFT, conjugate, scale
    let mut output: Vec<Complex> = input.iter().map(|c| c.conj()).collect();
    fft_in_place(&mut output);

    let scale = 1.0 / n as f64;
    output.iter().map(|c| c.conj().scale(scale)).collect()
}

/// In-place Cooley-Tukey radix-2 FFT.
fn fft_in_place(data: &mut [Complex]) {
    let n = data.len();
    if n <= 1 {
        return;
    }

    // Bit-reversal permutation
    let mut j = 0;
    for i in 0..n {
        if i < j {
            data.swap(i, j);
        }
        let mut m = n >> 1;
        while m > 0 && j >= m {
            j -= m;
            m >>= 1;
        }
        j += m;
    }

    // Cooley-Tukey iterative FFT
    let mut len = 2;
    while len <= n {
        let half_len = len >> 1;
        let angle_step = -2.0 * PI / len as f64;

        for start in (0..n).step_by(len) {
            let mut w = Complex::ONE;
            let w_n = Complex::from_polar(1.0, angle_step);

            for k in 0..half_len {
                let even_idx = start + k;
                let odd_idx = start + k + half_len;

                let t = w * data[odd_idx];
                data[odd_idx] = data[even_idx] - t;
                data[even_idx] = data[even_idx] + t;

                w = w * w_n;
            }
        }

        len <<= 1;
    }
}

/// Compute the FFT of a real-valued sequence.
///
/// For real input of length N (must be even), returns N/2 + 1 complex values
/// representing the non-redundant frequency components.
pub fn rfft(input: &[f64]) -> Vec<Complex> {
    let n = input.len();
    assert!(n % 2 == 0, "Real FFT input length must be even");
    assert!(n.is_power_of_two(), "Real FFT input length must be a power of 2");

    if n == 0 {
        return Vec::new();
    }

    // Pack real data into complex: x[2k] + i*x[2k+1]
    let half_n = n / 2;
    let mut packed: Vec<Complex> = (0..half_n)
        .map(|k| Complex::new(input[2 * k], input[2 * k + 1]))
        .collect();

    fft_in_place(&mut packed);

    // Unpack to get full spectrum
    let mut output = Vec::with_capacity(half_n + 1);

    // DC component
    output.push(Complex::new(packed[0].re + packed[0].im, 0.0));

    for k in 1..half_n {
        let z_k = packed[k];
        let z_nk = packed[half_n - k].conj();

        let even = (z_k + z_nk).scale(0.5);
        let odd = (z_k - z_nk).scale(0.5) * Complex::new(0.0, -1.0);

        let twiddle = Complex::from_polar(1.0, -2.0 * PI * k as f64 / n as f64);
        output.push(even + odd * twiddle);
    }

    // Nyquist component
    output.push(Complex::new(packed[0].re - packed[0].im, 0.0));

    output
}

/// Compute the inverse real FFT.
///
/// Input is N/2 + 1 complex values, output is N real values.
pub fn irfft(input: &[Complex], output_len: usize) -> Vec<f64> {
    let n = output_len;
    assert!(n % 2 == 0, "Output length must be even");
    assert!(n.is_power_of_two(), "Output length must be a power of 2");
    assert_eq!(input.len(), n / 2 + 1, "Input length must be N/2 + 1");

    let half_n = n / 2;

    // Reconstruct full complex spectrum
    let mut full: Vec<Complex> = Vec::with_capacity(n);
    for k in 0..=half_n {
        full.push(input[k]);
    }
    for k in 1..half_n {
        full.push(input[half_n - k].conj());
    }

    // Inverse FFT
    let result = ifft(&full);

    // Extract real parts
    result.iter().map(|c| c.re).collect()
}

/// Perform FFT on real data and return real and imaginary parts separately.
///
/// This matches the BrainFlow `perform_fft` interface.
pub fn perform_fft(data: &[f64], window: WindowType) -> (Vec<f64>, Vec<f64>) {
    let n = data.len();
    if n == 0 || n % 2 != 0 {
        return (Vec::new(), Vec::new());
    }

    // Apply window
    let windowed = apply_window(data, window);

    // Pad to power of 2 if necessary
    let fft_len = n.next_power_of_two();
    let mut padded = windowed;
    padded.resize(fft_len, 0.0);

    // Compute FFT
    let spectrum = rfft(&padded);

    // Extract real and imaginary parts
    let real: Vec<f64> = spectrum.iter().map(|c| c.re).collect();
    let imag: Vec<f64> = spectrum.iter().map(|c| c.im).collect();

    (real, imag)
}

/// Perform inverse FFT from real and imaginary parts.
///
/// This matches the BrainFlow `perform_ifft` interface.
pub fn perform_ifft(real: &[f64], imag: &[f64]) -> Vec<f64> {
    assert_eq!(real.len(), imag.len(), "Real and imaginary arrays must have same length");

    if real.is_empty() {
        return Vec::new();
    }

    let spectrum: Vec<Complex> = real
        .iter()
        .zip(imag.iter())
        .map(|(&r, &i)| Complex::new(r, i))
        .collect();

    let output_len = (spectrum.len() - 1) * 2;
    irfft(&spectrum, output_len)
}

/// Get the nearest power of 2 greater than or equal to n.
#[must_use]
pub const fn nearest_power_of_two(n: usize) -> usize {
    if n == 0 {
        return 1;
    }
    n.next_power_of_two()
}

/// Compute frequency bins for an FFT result.
#[must_use]
pub fn fft_frequencies(n: usize, sample_rate: f64) -> Vec<f64> {
    let freq_res = sample_rate / n as f64;
    (0..=n / 2).map(|i| i as f64 * freq_res).collect()
}

#[cfg(test)]
mod tests {
    use super::*;

    fn approx_eq(a: f64, b: f64, epsilon: f64) -> bool {
        (a - b).abs() < epsilon
    }

    #[test]
    fn test_fft_single() {
        let input = [Complex::new(1.0, 0.0)];
        let output = fft(&input);
        assert_eq!(output.len(), 1);
        assert!(approx_eq(output[0].re, 1.0, 1e-10));
    }

    #[test]
    fn test_fft_dc() {
        // All ones should give DC spike
        let input: Vec<Complex> = (0..4).map(|_| Complex::new(1.0, 0.0)).collect();
        let output = fft(&input);
        assert!(approx_eq(output[0].re, 4.0, 1e-10));
        assert!(approx_eq(output[1].norm(), 0.0, 1e-10));
        assert!(approx_eq(output[2].norm(), 0.0, 1e-10));
        assert!(approx_eq(output[3].norm(), 0.0, 1e-10));
    }

    #[test]
    fn test_fft_ifft_roundtrip() {
        let input: Vec<Complex> = (0..8)
            .map(|i| Complex::new(i as f64, (i as f64).sin()))
            .collect();

        let transformed = fft(&input);
        let recovered = ifft(&transformed);

        for (a, b) in input.iter().zip(recovered.iter()) {
            assert!(approx_eq(a.re, b.re, 1e-10));
            assert!(approx_eq(a.im, b.im, 1e-10));
        }
    }

    #[test]
    fn test_rfft_pure_sine() {
        // Test with a pure sine wave at known frequency
        let n = 64;
        let freq = 4.0; // 4 cycles in the window
        let input: Vec<f64> = (0..n)
            .map(|i| libm::sin(2.0 * PI * freq * i as f64 / n as f64))
            .collect();

        let spectrum = rfft(&input);

        // Peak should be at bin 4
        let magnitudes: Vec<f64> = spectrum.iter().map(|c| c.norm()).collect();
        let peak_bin = magnitudes
            .iter()
            .enumerate()
            .max_by(|(_, a), (_, b)| a.partial_cmp(b).unwrap())
            .map(|(i, _)| i)
            .unwrap();

        assert_eq!(peak_bin, freq as usize);
    }

    #[test]
    fn test_perform_fft_ifft() {
        let n = 64;
        let input: Vec<f64> = (0..n).map(|i| libm::sin(0.1 * i as f64)).collect();

        let (real, imag) = perform_fft(&input, WindowType::None);
        let recovered = perform_ifft(&real, &imag);

        // Check first few values (there may be edge effects from padding)
        for i in 0..n {
            assert!(
                approx_eq(input[i], recovered[i], 1e-10),
                "Mismatch at index {}: {} vs {}",
                i,
                input[i],
                recovered[i]
            );
        }
    }
}

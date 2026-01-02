//! Pure Rust digital filter design for BrainFlow.
//!
//! This crate provides:
//! - IIR filter design (Butterworth, Chebyshev Type I, Bessel)
//! - Low-pass, high-pass, band-pass, band-stop filters
//! - Zero-phase filtering
//! - Environmental noise removal (50/60 Hz)

#![no_std]

extern crate alloc;

use alloc::vec::Vec;
use brainflow_math::complex::Complex;
use brainflow_math::consts::PI;

pub mod biquad;
pub mod design;
pub mod iir;

pub use biquad::BiquadFilter;
pub use design::{FilterType, FilterDesign};
pub use iir::{IirFilter, FilterMode};

/// Detrend types matching BrainFlow constants.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum DetrendType {
    /// No detrending.
    #[default]
    None,
    /// Remove DC offset (mean).
    Constant,
    /// Remove linear trend.
    Linear,
}

/// Noise types for environmental noise removal.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum NoiseType {
    /// 50 Hz (European power line frequency).
    Fifty,
    /// 60 Hz (American power line frequency).
    Sixty,
    /// Both 50 Hz and 60 Hz.
    FiftyAndSixty,
}

/// Perform low-pass filtering.
///
/// # Arguments
/// * `data` - Input signal (modified in place)
/// * `sampling_rate` - Sampling frequency in Hz
/// * `cutoff` - Cutoff frequency in Hz
/// * `order` - Filter order (1-8)
/// * `filter_type` - Type of filter (Butterworth, Chebyshev, Bessel)
/// * `ripple` - Passband ripple in dB (only for Chebyshev)
pub fn perform_lowpass(
    data: &mut [f64],
    sampling_rate: f64,
    cutoff: f64,
    order: usize,
    filter_type: FilterType,
    ripple: f64,
) {
    let filter = IirFilter::lowpass(sampling_rate, cutoff, order, filter_type, ripple);
    filter.apply(data);
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
    let filter = IirFilter::highpass(sampling_rate, cutoff, order, filter_type, ripple);
    filter.apply(data);
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
    let center = (low_cutoff + high_cutoff) / 2.0;
    let bandwidth = high_cutoff - low_cutoff;
    let filter = IirFilter::bandpass(sampling_rate, center, bandwidth, order, filter_type, ripple);
    filter.apply(data);
}

/// Perform band-stop (notch) filtering.
pub fn perform_bandstop(
    data: &mut [f64],
    sampling_rate: f64,
    low_cutoff: f64,
    high_cutoff: f64,
    order: usize,
    filter_type: FilterType,
    ripple: f64,
) {
    let center = (low_cutoff + high_cutoff) / 2.0;
    let bandwidth = high_cutoff - low_cutoff;
    let filter = IirFilter::bandstop(sampling_rate, center, bandwidth, order, filter_type, ripple);
    filter.apply(data);
}

/// Perform zero-phase low-pass filtering (forward-backward).
pub fn perform_lowpass_zero_phase(
    data: &mut [f64],
    sampling_rate: f64,
    cutoff: f64,
    order: usize,
    filter_type: FilterType,
    ripple: f64,
) {
    let filter = IirFilter::lowpass(sampling_rate, cutoff, order, filter_type, ripple);
    filter.apply_zero_phase(data);
}

/// Perform zero-phase high-pass filtering.
pub fn perform_highpass_zero_phase(
    data: &mut [f64],
    sampling_rate: f64,
    cutoff: f64,
    order: usize,
    filter_type: FilterType,
    ripple: f64,
) {
    let filter = IirFilter::highpass(sampling_rate, cutoff, order, filter_type, ripple);
    filter.apply_zero_phase(data);
}

/// Perform zero-phase band-pass filtering.
pub fn perform_bandpass_zero_phase(
    data: &mut [f64],
    sampling_rate: f64,
    low_cutoff: f64,
    high_cutoff: f64,
    order: usize,
    filter_type: FilterType,
    ripple: f64,
) {
    let center = (low_cutoff + high_cutoff) / 2.0;
    let bandwidth = high_cutoff - low_cutoff;
    let filter = IirFilter::bandpass(sampling_rate, center, bandwidth, order, filter_type, ripple);
    filter.apply_zero_phase(data);
}

/// Perform zero-phase band-stop filtering.
pub fn perform_bandstop_zero_phase(
    data: &mut [f64],
    sampling_rate: f64,
    low_cutoff: f64,
    high_cutoff: f64,
    order: usize,
    filter_type: FilterType,
    ripple: f64,
) {
    let center = (low_cutoff + high_cutoff) / 2.0;
    let bandwidth = high_cutoff - low_cutoff;
    let filter = IirFilter::bandstop(sampling_rate, center, bandwidth, order, filter_type, ripple);
    filter.apply_zero_phase(data);
}

/// Remove environmental noise (power line interference).
pub fn remove_environmental_noise(data: &mut [f64], sampling_rate: f64, noise_type: NoiseType) {
    match noise_type {
        NoiseType::Fifty => {
            // 50 Hz notch: 48-52 Hz bandstop
            perform_bandstop(data, sampling_rate, 48.0, 52.0, 4, FilterType::Butterworth, 0.0);
        }
        NoiseType::Sixty => {
            // 60 Hz notch: 58-62 Hz bandstop
            perform_bandstop(data, sampling_rate, 58.0, 62.0, 4, FilterType::Butterworth, 0.0);
        }
        NoiseType::FiftyAndSixty => {
            perform_bandstop(data, sampling_rate, 48.0, 52.0, 4, FilterType::Butterworth, 0.0);
            perform_bandstop(data, sampling_rate, 58.0, 62.0, 4, FilterType::Butterworth, 0.0);
        }
    }
}

/// Detrend a signal.
pub fn detrend(data: &mut [f64], detrend_type: DetrendType) {
    match detrend_type {
        DetrendType::None => {}
        DetrendType::Constant => {
            let mean = brainflow_math::stats::mean(data);
            for x in data.iter_mut() {
                *x -= mean;
            }
        }
        DetrendType::Linear => {
            let detrended = brainflow_math::stats::detrend_linear(data);
            data.copy_from_slice(&detrended);
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_lowpass_removes_high_freq() {
        // Create signal with low and high frequency components
        let n = 256;
        let fs = 256.0;
        let mut data: Vec<f64> = (0..n)
            .map(|i| {
                let t = i as f64 / fs;
                libm::sin(2.0 * PI * 5.0 * t) + libm::sin(2.0 * PI * 50.0 * t)
            })
            .collect();

        perform_lowpass(&mut data, fs, 10.0, 4, FilterType::Butterworth, 0.0);

        // High frequency component should be attenuated
        // Check that the signal is mostly the low frequency component
        let mut max_val = 0.0f64;
        for &x in &data[n/4..3*n/4] {
            max_val = max_val.max(libm::fabs(x));
        }
        // Should be around 1.0 (the low freq component) not 2.0
        assert!(max_val < 1.5);
    }
}

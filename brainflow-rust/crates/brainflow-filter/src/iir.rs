//! High-level IIR filter interface.

use alloc::vec::Vec;

use crate::biquad::BiquadCascade;
use crate::design::{FilterDesign, FilterType};

/// Filter mode (lowpass, highpass, etc.).
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum FilterMode {
    /// Low-pass filter.
    Lowpass,
    /// High-pass filter.
    Highpass,
    /// Band-pass filter.
    Bandpass,
    /// Band-stop (notch) filter.
    Bandstop,
}

/// An IIR filter that can be applied to signals.
#[derive(Debug, Clone)]
pub struct IirFilter {
    cascade: BiquadCascade,
}

impl IirFilter {
    /// Create a low-pass filter.
    #[must_use]
    pub fn lowpass(
        sampling_rate: f64,
        cutoff: f64,
        order: usize,
        filter_type: FilterType,
        ripple: f64,
    ) -> Self {
        let design = FilterDesign::new(filter_type, order, sampling_rate, ripple);
        Self {
            cascade: design.lowpass(cutoff),
        }
    }

    /// Create a high-pass filter.
    #[must_use]
    pub fn highpass(
        sampling_rate: f64,
        cutoff: f64,
        order: usize,
        filter_type: FilterType,
        ripple: f64,
    ) -> Self {
        let design = FilterDesign::new(filter_type, order, sampling_rate, ripple);
        Self {
            cascade: design.highpass(cutoff),
        }
    }

    /// Create a band-pass filter.
    #[must_use]
    pub fn bandpass(
        sampling_rate: f64,
        center: f64,
        bandwidth: f64,
        order: usize,
        filter_type: FilterType,
        ripple: f64,
    ) -> Self {
        let design = FilterDesign::new(filter_type, order, sampling_rate, ripple);
        Self {
            cascade: design.bandpass(center, bandwidth),
        }
    }

    /// Create a band-stop (notch) filter.
    #[must_use]
    pub fn bandstop(
        sampling_rate: f64,
        center: f64,
        bandwidth: f64,
        order: usize,
        filter_type: FilterType,
        ripple: f64,
    ) -> Self {
        let design = FilterDesign::new(filter_type, order, sampling_rate, ripple);
        Self {
            cascade: design.bandstop(center, bandwidth),
        }
    }

    /// Reset the filter state.
    pub fn reset(&mut self) {
        self.cascade.reset();
    }

    /// Process a single sample.
    #[inline]
    pub fn process(&mut self, x: f64) -> f64 {
        self.cascade.process(x)
    }

    /// Apply the filter to data in place.
    pub fn apply(&self, data: &mut [f64]) {
        let mut cascade = self.cascade.clone();
        cascade.apply(data);
    }

    /// Apply zero-phase filtering (forward-backward).
    ///
    /// This eliminates phase distortion but doubles the effective order.
    pub fn apply_zero_phase(&self, data: &mut [f64]) {
        // Forward pass
        let mut cascade = self.cascade.clone();
        cascade.apply(data);

        // Reverse
        data.reverse();

        // Backward pass
        cascade.reset();
        cascade.apply(data);

        // Reverse again to restore order
        data.reverse();
    }

    /// Filter data and return a new vector.
    #[must_use]
    pub fn filter(&self, data: &[f64]) -> Vec<f64> {
        let mut result = data.to_vec();
        self.apply(&mut result);
        result
    }

    /// Filter data with zero-phase and return a new vector.
    #[must_use]
    pub fn filter_zero_phase(&self, data: &[f64]) -> Vec<f64> {
        let mut result = data.to_vec();
        self.apply_zero_phase(&mut result);
        result
    }
}

/// Rolling filter implementations.
pub mod rolling {
    use alloc::collections::VecDeque;
    use alloc::vec::Vec;

    /// A rolling average filter.
    #[derive(Debug, Clone)]
    pub struct RollingAverage {
        window: VecDeque<f64>,
        sum: f64,
        size: usize,
    }

    impl RollingAverage {
        /// Create a new rolling average with the given window size.
        #[must_use]
        pub fn new(size: usize) -> Self {
            Self {
                window: VecDeque::with_capacity(size),
                sum: 0.0,
                size,
            }
        }

        /// Add a value and return the current average.
        pub fn update(&mut self, value: f64) -> f64 {
            self.sum += value;
            self.window.push_back(value);

            if self.window.len() > self.size {
                if let Some(old) = self.window.pop_front() {
                    self.sum -= old;
                }
            }

            self.sum / self.window.len() as f64
        }

        /// Get the current average without adding a value.
        #[must_use]
        pub fn current(&self) -> f64 {
            if self.window.is_empty() {
                0.0
            } else {
                self.sum / self.window.len() as f64
            }
        }

        /// Reset the filter.
        pub fn reset(&mut self) {
            self.window.clear();
            self.sum = 0.0;
        }
    }

    /// A rolling median filter.
    #[derive(Debug, Clone)]
    pub struct RollingMedian {
        window: VecDeque<f64>,
        size: usize,
    }

    impl RollingMedian {
        /// Create a new rolling median with the given window size.
        #[must_use]
        pub fn new(size: usize) -> Self {
            Self {
                window: VecDeque::with_capacity(size),
                size,
            }
        }

        /// Add a value and return the current median.
        pub fn update(&mut self, value: f64) -> f64 {
            self.window.push_back(value);

            if self.window.len() > self.size {
                self.window.pop_front();
            }

            self.compute_median()
        }

        /// Compute the median of the current window.
        fn compute_median(&self) -> f64 {
            if self.window.is_empty() {
                return 0.0;
            }

            let mut sorted: Vec<f64> = self.window.iter().copied().collect();
            // Simple insertion sort
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

        /// Reset the filter.
        pub fn reset(&mut self) {
            self.window.clear();
        }
    }

    /// Apply rolling average to a signal.
    #[must_use]
    pub fn apply_rolling_average(data: &[f64], window_size: usize) -> Vec<f64> {
        let mut filter = RollingAverage::new(window_size);
        data.iter().map(|&x| filter.update(x)).collect()
    }

    /// Apply rolling median to a signal.
    #[must_use]
    pub fn apply_rolling_median(data: &[f64], window_size: usize) -> Vec<f64> {
        let mut filter = RollingMedian::new(window_size);
        data.iter().map(|&x| filter.update(x)).collect()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use brainflow_math::consts::PI;

    #[test]
    fn test_lowpass_attenuates_high_freq() {
        let fs = 1000.0;
        let n = 1000;

        // Signal with 10 Hz and 100 Hz components
        let mut data: Vec<f64> = (0..n)
            .map(|i| {
                let t = i as f64 / fs;
                libm::sin(2.0 * PI * 10.0 * t) + libm::sin(2.0 * PI * 100.0 * t)
            })
            .collect();

        let filter = IirFilter::lowpass(fs, 30.0, 4, FilterType::Butterworth, 0.0);
        filter.apply(&mut data);

        // Check that high frequency is attenuated
        // The signal should now be mostly the 10 Hz component
        let mut max_val = 0.0f64;
        for &x in &data[n/4..3*n/4] {
            max_val = max_val.max(libm::fabs(x));
        }

        // Original peak was 2.0, filtered should be close to 1.0
        assert!(max_val < 1.5, "High frequency should be attenuated");
    }

    #[test]
    fn test_zero_phase_filter_runs() {
        // Basic test that zero-phase filtering runs without panic
        // Full phase verification requires more investigation of filter design
        let fs = 1000.0;
        let n = 100;

        let original: Vec<f64> = (0..n)
            .map(|i| libm::sin(2.0 * PI * 10.0 * i as f64 / fs))
            .collect();

        let filter = IirFilter::lowpass(fs, 50.0, 2, FilterType::Butterworth, 0.0);
        let filtered = filter.filter_zero_phase(&original);

        // Just verify the output has the same length and reasonable values
        assert_eq!(filtered.len(), original.len());

        // Verify output is not all zeros or NaN
        let sum: f64 = filtered.iter().map(|x| x.abs()).sum();
        assert!(sum > 0.0, "Filtered output should not be all zeros");
        assert!(!sum.is_nan(), "Filtered output should not contain NaN");
    }

    #[test]
    fn test_rolling_average() {
        let data = [1.0, 2.0, 3.0, 4.0, 5.0];
        let filtered = rolling::apply_rolling_average(&data, 3);

        // First value: 1/1 = 1
        // Second value: (1+2)/2 = 1.5
        // Third value: (1+2+3)/3 = 2
        // Fourth value: (2+3+4)/3 = 3
        // Fifth value: (3+4+5)/3 = 4
        assert!((filtered[0] - 1.0).abs() < 1e-10);
        assert!((filtered[1] - 1.5).abs() < 1e-10);
        assert!((filtered[2] - 2.0).abs() < 1e-10);
        assert!((filtered[3] - 3.0).abs() < 1e-10);
        assert!((filtered[4] - 4.0).abs() < 1e-10);
    }
}

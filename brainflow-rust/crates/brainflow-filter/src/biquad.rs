//! Biquad (second-order section) filter implementation.

use alloc::vec::Vec;

/// A biquad (second-order IIR) filter section.
///
/// Transfer function: H(z) = (b0 + b1*z^-1 + b2*z^-2) / (1 + a1*z^-1 + a2*z^-2)
#[derive(Debug, Clone)]
pub struct BiquadFilter {
    /// Numerator coefficients [b0, b1, b2]
    pub b: [f64; 3],
    /// Denominator coefficients [a1, a2] (a0 is normalized to 1)
    pub a: [f64; 2],
    /// State variables for Direct Form II
    state: [f64; 2],
}

impl BiquadFilter {
    /// Create a new biquad filter from coefficients.
    ///
    /// Coefficients are normalized so that a0 = 1.
    #[must_use]
    pub fn new(b0: f64, b1: f64, b2: f64, a0: f64, a1: f64, a2: f64) -> Self {
        let norm = 1.0 / a0;
        Self {
            b: [b0 * norm, b1 * norm, b2 * norm],
            a: [a1 * norm, a2 * norm],
            state: [0.0; 2],
        }
    }

    /// Create a biquad from normalized coefficients (a0 = 1).
    #[must_use]
    pub fn from_normalized(b: [f64; 3], a: [f64; 2]) -> Self {
        Self { b, a, state: [0.0; 2] }
    }

    /// Reset the filter state.
    pub fn reset(&mut self) {
        self.state = [0.0; 2];
    }

    /// Process a single sample using Direct Form II Transposed.
    #[inline]
    pub fn process(&mut self, x: f64) -> f64 {
        let y = self.b[0] * x + self.state[0];
        self.state[0] = self.b[1] * x - self.a[0] * y + self.state[1];
        self.state[1] = self.b[2] * x - self.a[1] * y;
        y
    }

    /// Filter an entire buffer in place.
    pub fn apply(&mut self, data: &mut [f64]) {
        for x in data.iter_mut() {
            *x = self.process(*x);
        }
    }

    /// Filter a buffer and return a new vector.
    #[must_use]
    pub fn filter(&mut self, data: &[f64]) -> Vec<f64> {
        data.iter().map(|&x| self.process(x)).collect()
    }
}

/// A cascade of biquad sections for higher-order filters.
#[derive(Debug, Clone)]
pub struct BiquadCascade {
    sections: Vec<BiquadFilter>,
}

impl BiquadCascade {
    /// Create an empty cascade.
    #[must_use]
    pub fn new() -> Self {
        Self {
            sections: Vec::new(),
        }
    }

    /// Create a cascade from a list of biquad sections.
    #[must_use]
    pub fn from_sections(sections: Vec<BiquadFilter>) -> Self {
        Self { sections }
    }

    /// Add a biquad section to the cascade.
    pub fn add_section(&mut self, section: BiquadFilter) {
        self.sections.push(section);
    }

    /// Get the number of sections.
    #[must_use]
    pub fn len(&self) -> usize {
        self.sections.len()
    }

    /// Check if the cascade is empty.
    #[must_use]
    pub fn is_empty(&self) -> bool {
        self.sections.is_empty()
    }

    /// Reset all section states.
    pub fn reset(&mut self) {
        for section in &mut self.sections {
            section.reset();
        }
    }

    /// Process a single sample through all sections.
    #[inline]
    pub fn process(&mut self, x: f64) -> f64 {
        let mut y = x;
        for section in &mut self.sections {
            y = section.process(y);
        }
        y
    }

    /// Filter an entire buffer in place.
    pub fn apply(&mut self, data: &mut [f64]) {
        for x in data.iter_mut() {
            *x = self.process(*x);
        }
    }

    /// Filter a buffer and return a new vector.
    #[must_use]
    pub fn filter(&mut self, data: &[f64]) -> Vec<f64> {
        data.iter().map(|&x| self.process(x)).collect()
    }

    /// Get mutable access to sections (for gain normalization).
    pub fn sections_mut(&mut self) -> &mut Vec<BiquadFilter> {
        &mut self.sections
    }

    /// Get immutable access to sections.
    pub fn sections(&self) -> &[BiquadFilter] {
        &self.sections
    }
}

impl Default for BiquadCascade {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_passthrough() {
        // Identity filter: y = x
        let mut bq = BiquadFilter::from_normalized([1.0, 0.0, 0.0], [0.0, 0.0]);

        let input = [1.0, 2.0, 3.0, 4.0, 5.0];
        for &x in &input {
            let y = bq.process(x);
            assert!((y - x).abs() < 1e-10);
        }
    }

    #[test]
    fn test_dc_gain() {
        // Simple lowpass: at DC (z=1), gain = (b0+b1+b2)/(1+a1+a2)
        let b = [0.25, 0.5, 0.25];
        let a = [-0.5, 0.2];
        let mut bq = BiquadFilter::from_normalized(b, a);

        // DC gain
        let dc_gain = (b[0] + b[1] + b[2]) / (1.0 + a[0] + a[1]);

        // Feed constant input until steady state
        let mut y = 0.0;
        for _ in 0..100 {
            y = bq.process(1.0);
        }

        assert!((y - dc_gain).abs() < 0.01);
    }
}

//! Filter design algorithms.
//!
//! Implements analog prototype to digital filter conversion using the bilinear transform.

use alloc::vec::Vec;
use brainflow_math::complex::Complex;
use brainflow_math::consts::PI;

use crate::biquad::{BiquadFilter, BiquadCascade};

/// Filter design types.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum FilterType {
    /// Butterworth: maximally flat passband.
    #[default]
    Butterworth,
    /// Chebyshev Type I: ripple in passband, steeper rolloff.
    ChebyshevI,
    /// Bessel: maximally flat group delay.
    Bessel,
}

/// Filter design specification.
#[derive(Debug, Clone)]
pub struct FilterDesign {
    /// Filter type
    pub filter_type: FilterType,
    /// Filter order
    pub order: usize,
    /// Sampling rate in Hz
    pub sampling_rate: f64,
    /// Passband ripple in dB (Chebyshev only)
    pub ripple: f64,
}

impl FilterDesign {
    /// Create a new filter design specification.
    #[must_use]
    pub fn new(filter_type: FilterType, order: usize, sampling_rate: f64, ripple: f64) -> Self {
        Self {
            filter_type,
            order: order.min(8), // Maximum order 8 as per BrainFlow
            sampling_rate,
            ripple,
        }
    }

    /// Design a lowpass filter.
    #[must_use]
    pub fn lowpass(&self, cutoff: f64) -> BiquadCascade {
        let warped = warp_frequency(cutoff, self.sampling_rate);
        let poles = self.analog_poles(warped);
        let zeros = analog_lowpass_zeros(self.order);
        let gain = self.analog_lowpass_gain(warped);

        poles_zeros_to_cascade(&poles, &zeros, gain, self.sampling_rate)
    }

    /// Design a highpass filter.
    #[must_use]
    pub fn highpass(&self, cutoff: f64) -> BiquadCascade {
        let warped = warp_frequency(cutoff, self.sampling_rate);

        // Transform lowpass prototype to highpass: s -> 1/s
        let lp_poles = self.analog_poles(1.0);
        let poles: Vec<Complex> = lp_poles.iter().map(|p| Complex::from_real(warped) / *p).collect();

        // Highpass zeros are all at origin in analog domain
        let zeros: Vec<Complex> = (0..self.order).map(|_| Complex::ZERO).collect();

        let gain = self.analog_highpass_gain(warped);

        poles_zeros_to_cascade(&poles, &zeros, gain, self.sampling_rate)
    }

    /// Design a bandpass filter.
    #[must_use]
    pub fn bandpass(&self, center: f64, bandwidth: f64) -> BiquadCascade {
        let w0 = warp_frequency(center, self.sampling_rate);
        let bw = warp_frequency(center + bandwidth / 2.0, self.sampling_rate)
            - warp_frequency(center - bandwidth / 2.0, self.sampling_rate);

        let lp_poles = self.analog_poles(1.0);

        // Lowpass to bandpass transformation doubles the order
        let mut bp_poles = Vec::with_capacity(2 * self.order);
        for p in &lp_poles {
            let (p1, p2) = lp_to_bp_pole(*p, w0, bw);
            bp_poles.push(p1);
            bp_poles.push(p2);
        }

        // Bandpass zeros: order zeros at origin, order zeros at infinity
        let zeros: Vec<Complex> = (0..self.order).map(|_| Complex::ZERO).collect();

        let gain = self.analog_bandpass_gain(w0, bw);

        poles_zeros_to_cascade(&bp_poles, &zeros, gain, self.sampling_rate)
    }

    /// Design a bandstop (notch) filter.
    #[must_use]
    pub fn bandstop(&self, center: f64, bandwidth: f64) -> BiquadCascade {
        let w0 = warp_frequency(center, self.sampling_rate);
        let bw = warp_frequency(center + bandwidth / 2.0, self.sampling_rate)
            - warp_frequency(center - bandwidth / 2.0, self.sampling_rate);

        let lp_poles = self.analog_poles(1.0);

        // Lowpass to bandstop transformation
        let mut bs_poles = Vec::with_capacity(2 * self.order);
        for p in &lp_poles {
            let (p1, p2) = lp_to_bs_pole(*p, w0, bw);
            bs_poles.push(p1);
            bs_poles.push(p2);
        }

        // Bandstop zeros are at ±jω0
        let mut zeros = Vec::with_capacity(2 * self.order);
        for _ in 0..self.order {
            zeros.push(Complex::new(0.0, w0));
            zeros.push(Complex::new(0.0, -w0));
        }

        let gain = 1.0; // Normalize later

        poles_zeros_to_cascade(&bs_poles, &zeros, gain, self.sampling_rate)
    }

    /// Get analog prototype poles.
    fn analog_poles(&self, freq_scale: f64) -> Vec<Complex> {
        match self.filter_type {
            FilterType::Butterworth => butterworth_poles(self.order, freq_scale),
            FilterType::ChebyshevI => chebyshev_poles(self.order, self.ripple, freq_scale),
            FilterType::Bessel => bessel_poles(self.order, freq_scale),
        }
    }

    /// Compute analog lowpass gain.
    fn analog_lowpass_gain(&self, warped: f64) -> f64 {
        match self.filter_type {
            FilterType::Butterworth => libm::pow(warped, self.order as f64),
            FilterType::ChebyshevI => {
                let epsilon = libm::sqrt(libm::pow(10.0, self.ripple / 10.0) - 1.0);
                libm::pow(warped, self.order as f64) / libm::sqrt(1.0 + epsilon * epsilon)
            }
            FilterType::Bessel => libm::pow(warped, self.order as f64),
        }
    }

    /// Compute analog highpass gain.
    fn analog_highpass_gain(&self, _warped: f64) -> f64 {
        1.0
    }

    /// Compute analog bandpass gain.
    fn analog_bandpass_gain(&self, _w0: f64, bw: f64) -> f64 {
        libm::pow(bw, self.order as f64)
    }
}

/// Warp analog frequency to account for bilinear transform.
fn warp_frequency(freq: f64, sample_rate: f64) -> f64 {
    2.0 * sample_rate * libm::tan(PI * freq / sample_rate)
}

/// Butterworth analog prototype poles.
fn butterworth_poles(order: usize, freq_scale: f64) -> Vec<Complex> {
    (0..order)
        .map(|k| {
            let angle = PI * (2 * k + order + 1) as f64 / (2 * order) as f64;
            Complex::from_polar(freq_scale, angle)
        })
        .collect()
}

/// Chebyshev Type I analog prototype poles.
fn chebyshev_poles(order: usize, ripple_db: f64, freq_scale: f64) -> Vec<Complex> {
    let epsilon = libm::sqrt(libm::pow(10.0, ripple_db / 10.0) - 1.0);
    let mu = libm::asinh(1.0 / epsilon) / order as f64;

    (0..order)
        .map(|k| {
            let theta = PI * (2 * k + 1) as f64 / (2 * order) as f64;
            let real = -libm::sinh(mu) * libm::sin(theta);
            let imag = libm::cosh(mu) * libm::cos(theta);
            Complex::new(real * freq_scale, imag * freq_scale)
        })
        .collect()
}

/// Bessel analog prototype poles (approximated using recurrence).
fn bessel_poles(order: usize, freq_scale: f64) -> Vec<Complex> {
    // Bessel polynomial roots (pre-computed for orders 1-8)
    // These are normalized for unit delay at DC
    let poles: &[&[Complex]] = &[
        // Order 1
        &[Complex::new(-1.0, 0.0)],
        // Order 2
        &[Complex::new(-1.1016, 0.6368), Complex::new(-1.1016, -0.6368)],
        // Order 3
        &[
            Complex::new(-1.0474, 0.9992),
            Complex::new(-1.0474, -0.9992),
            Complex::new(-1.3226, 0.0),
        ],
        // Order 4
        &[
            Complex::new(-0.9952, 1.2571),
            Complex::new(-0.9952, -1.2571),
            Complex::new(-1.3700, 0.4102),
            Complex::new(-1.3700, -0.4102),
        ],
        // Order 5
        &[
            Complex::new(-0.9576, 1.4711),
            Complex::new(-0.9576, -1.4711),
            Complex::new(-1.3808, 0.7179),
            Complex::new(-1.3808, -0.7179),
            Complex::new(-1.5023, 0.0),
        ],
        // Order 6
        &[
            Complex::new(-0.9306, 1.6618),
            Complex::new(-0.9306, -1.6618),
            Complex::new(-1.3818, 0.9714),
            Complex::new(-1.3818, -0.9714),
            Complex::new(-1.5714, 0.3213),
            Complex::new(-1.5714, -0.3213),
        ],
        // Order 7
        &[
            Complex::new(-0.9098, 1.8364),
            Complex::new(-0.9098, -1.8364),
            Complex::new(-1.3789, 1.1915),
            Complex::new(-1.3789, -1.1915),
            Complex::new(-1.6120, 0.5896),
            Complex::new(-1.6120, -0.5896),
            Complex::new(-1.6843, 0.0),
        ],
        // Order 8
        &[
            Complex::new(-0.8928, 1.9983),
            Complex::new(-0.8928, -1.9983),
            Complex::new(-1.3738, 1.3884),
            Complex::new(-1.3738, -1.3884),
            Complex::new(-1.6369, 0.8224),
            Complex::new(-1.6369, -0.8224),
            Complex::new(-1.7574, 0.2737),
            Complex::new(-1.7574, -0.2737),
        ],
    ];

    if order == 0 || order > 8 {
        return Vec::new();
    }

    poles[order - 1]
        .iter()
        .map(|p| p.scale(freq_scale))
        .collect()
}

/// Analog lowpass zeros (all at infinity, represented as empty for poles-only design).
fn analog_lowpass_zeros(_order: usize) -> Vec<Complex> {
    Vec::new() // Lowpass prototype has no finite zeros
}

/// Lowpass to bandpass pole transformation.
fn lp_to_bp_pole(p: Complex, w0: f64, bw: f64) -> (Complex, Complex) {
    let p_scaled = p.scale(bw / 2.0);
    let discriminant = p_scaled * p_scaled - Complex::from_real(w0 * w0);
    let sqrt_disc = discriminant.sqrt();

    (p_scaled + sqrt_disc, p_scaled - sqrt_disc)
}

/// Lowpass to bandstop pole transformation.
fn lp_to_bs_pole(p: Complex, w0: f64, bw: f64) -> (Complex, Complex) {
    let p_scaled = Complex::from_real(bw / 2.0) / p;
    let discriminant = p_scaled * p_scaled - Complex::from_real(w0 * w0);
    let sqrt_disc = discriminant.sqrt();

    (p_scaled + sqrt_disc, p_scaled - sqrt_disc)
}

/// Convert analog poles and zeros to digital biquad cascade using bilinear transform.
fn poles_zeros_to_cascade(
    poles: &[Complex],
    zeros: &[Complex],
    _gain: f64,
    sample_rate: f64,
) -> BiquadCascade {
    let t = 1.0 / sample_rate;
    let t2 = t / 2.0;

    // Bilinear transform: s = (2/T) * (z-1)/(z+1)
    let bilinear = |s: Complex| -> Complex {
        let num = Complex::ONE + s.scale(t2);
        let den = Complex::ONE - s.scale(t2);
        num / den
    };

    // Transform poles
    let z_poles: Vec<Complex> = poles.iter().map(|&p| bilinear(p)).collect();

    // Transform zeros (zeros at infinity map to z = -1)
    let mut z_zeros: Vec<Complex> = zeros.iter().map(|&z| bilinear(z)).collect();

    // Add zeros at z = -1 for each pole without a corresponding zero
    while z_zeros.len() < z_poles.len() {
        z_zeros.push(Complex::new(-1.0, 0.0));
    }

    // Group into second-order sections
    let mut cascade = BiquadCascade::new();

    let mut used_poles = alloc::vec![false; z_poles.len()];
    let mut used_zeros = alloc::vec![false; z_zeros.len()];

    // Pair complex conjugate poles
    let mut i = 0;
    while i < z_poles.len() {
        if used_poles[i] {
            i += 1;
            continue;
        }

        let p1 = z_poles[i];
        used_poles[i] = true;

        // Find conjugate pair if exists
        let mut p2_opt = None;
        if libm::fabs(p1.im) > 1e-10 {
            for j in (i + 1)..z_poles.len() {
                if !used_poles[j] && libm::fabs(z_poles[j].re - p1.re) < 1e-10
                    && libm::fabs(z_poles[j].im + p1.im) < 1e-10
                {
                    p2_opt = Some(j);
                    break;
                }
            }
        }

        // Find zeros to pair with
        let z1 = find_nearest_zero(&z_zeros, p1, &used_zeros);
        let z2 = if p2_opt.is_some() {
            find_nearest_zero(&z_zeros, z_poles[p2_opt.unwrap()], &used_zeros)
        } else {
            None
        };

        if let Some((z1_idx, z1_val)) = z1 {
            used_zeros[z1_idx] = true;

            if let Some(p2_idx) = p2_opt {
                used_poles[p2_idx] = true;
                let p2 = z_poles[p2_idx];

                if let Some((z2_idx, z2_val)) = z2 {
                    used_zeros[z2_idx] = true;

                    // Complex conjugate pair of poles and zeros
                    let section = create_sos(p1, Some(p2), z1_val, Some(z2_val));
                    cascade.add_section(section);
                } else {
                    // Complex poles, one zero
                    let section = create_sos(p1, Some(p2), z1_val, None);
                    cascade.add_section(section);
                }
            } else {
                // Real pole, one zero
                let section = create_sos(p1, None, z1_val, None);
                cascade.add_section(section);
            }
        } else if let Some(p2_idx) = p2_opt {
            used_poles[p2_idx] = true;
            let p2 = z_poles[p2_idx];

            // Complex poles, no zeros - add zeros at -1
            let section = create_sos(p1, Some(p2), Complex::new(-1.0, 0.0), Some(Complex::new(-1.0, 0.0)));
            cascade.add_section(section);
        } else {
            // Single real pole, add zero at -1
            let section = create_sos(p1, None, Complex::new(-1.0, 0.0), None);
            cascade.add_section(section);
        }

        i += 1;
    }

    // Normalize gain
    normalize_cascade_gain(&mut cascade);

    cascade
}

/// Find the nearest unused zero to a pole.
fn find_nearest_zero(
    zeros: &[Complex],
    pole: Complex,
    used: &[bool],
) -> Option<(usize, Complex)> {
    let mut best_idx = None;
    let mut best_dist = f64::INFINITY;

    for (i, &z) in zeros.iter().enumerate() {
        if used[i] {
            continue;
        }
        let dist = (z - pole).norm();
        if dist < best_dist {
            best_dist = dist;
            best_idx = Some(i);
        }
    }

    best_idx.map(|i| (i, zeros[i]))
}

/// Create a second-order section from poles and zeros.
fn create_sos(
    p1: Complex,
    p2: Option<Complex>,
    z1: Complex,
    z2: Option<Complex>,
) -> BiquadFilter {
    // Denominator: (z - p1)(z - p2) = z² - (p1+p2)z + p1*p2
    let (a1, a2) = if let Some(p2) = p2 {
        let sum = p1 + p2;
        let prod = p1 * p2;
        (-sum.re, prod.re)
    } else {
        (-p1.re, 0.0)
    };

    // Numerator: (z - z1)(z - z2) = z² - (z1+z2)z + z1*z2
    let (b0, b1, b2) = if let Some(z2) = z2 {
        let sum = z1 + z2;
        let prod = z1 * z2;
        (1.0, -sum.re, prod.re)
    } else {
        (1.0, -z1.re, 0.0)
    };

    BiquadFilter::from_normalized([b0, b1, b2], [a1, a2])
}

/// Normalize the cascade gain so DC or Nyquist gain is 1.
fn normalize_cascade_gain(cascade: &mut BiquadCascade) {
    // For lowpass, normalize at DC (z=1)
    // For highpass, normalize at Nyquist (z=-1)

    // Evaluate at z=1 (DC)
    let mut gain = 1.0;
    for section in cascade.sections().iter() {
        let num = section.b[0] + section.b[1] + section.b[2];
        let den = 1.0 + section.a[0] + section.a[1];
        if den.abs() > 1e-10 {
            gain *= num / den;
        }
    }

    if gain.abs() > 1e-10 && gain.abs() < 1e10 {
        let scale = 1.0 / gain;
        let scale_per_section = libm::pow(scale.abs(), 1.0 / cascade.len() as f64);

        for section in cascade.sections_mut().iter_mut() {
            section.b[0] *= scale_per_section;
            section.b[1] *= scale_per_section;
            section.b[2] *= scale_per_section;
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_butterworth_poles() {
        let poles = butterworth_poles(2, 1.0);
        assert_eq!(poles.len(), 2);
        // 2nd order Butterworth poles at -1/√2 ± j/√2
        let expected_real = -1.0 / libm::sqrt(2.0);
        assert!((poles[0].re - expected_real).abs() < 0.01);
    }

    #[test]
    fn test_lowpass_design() {
        let design = FilterDesign::new(FilterType::Butterworth, 2, 1000.0, 0.0);
        let cascade = design.lowpass(100.0);
        assert!(!cascade.is_empty());
    }
}

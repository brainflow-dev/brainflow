//! Complex number implementation.

use core::ops::{Add, AddAssign, Div, DivAssign, Mul, MulAssign, Neg, Sub, SubAssign};

/// A complex number with f64 real and imaginary parts.
#[derive(Debug, Clone, Copy, PartialEq, Default)]
pub struct Complex {
    /// Real part.
    pub re: f64,
    /// Imaginary part.
    pub im: f64,
}

impl Complex {
    /// Create a new complex number.
    #[must_use]
    #[inline]
    pub const fn new(re: f64, im: f64) -> Self {
        Self { re, im }
    }

    /// Create a purely real complex number.
    #[must_use]
    #[inline]
    pub const fn from_real(re: f64) -> Self {
        Self { re, im: 0.0 }
    }

    /// Create a purely imaginary complex number.
    #[must_use]
    #[inline]
    pub const fn from_imag(im: f64) -> Self {
        Self { re: 0.0, im }
    }

    /// Create a complex number from polar coordinates.
    #[must_use]
    #[inline]
    pub fn from_polar(r: f64, theta: f64) -> Self {
        Self {
            re: r * libm::cos(theta),
            im: r * libm::sin(theta),
        }
    }

    /// The complex zero (0 + 0i).
    pub const ZERO: Self = Self { re: 0.0, im: 0.0 };

    /// The complex one (1 + 0i).
    pub const ONE: Self = Self { re: 1.0, im: 0.0 };

    /// The imaginary unit (0 + 1i).
    pub const I: Self = Self { re: 0.0, im: 1.0 };

    /// Compute the magnitude (absolute value) squared.
    #[must_use]
    #[inline]
    pub fn norm_sqr(self) -> f64 {
        self.re * self.re + self.im * self.im
    }

    /// Compute the magnitude (absolute value).
    #[must_use]
    #[inline]
    pub fn norm(self) -> f64 {
        libm::sqrt(self.norm_sqr())
    }

    /// Compute the argument (phase angle) in radians.
    #[must_use]
    #[inline]
    pub fn arg(self) -> f64 {
        libm::atan2(self.im, self.re)
    }

    /// Compute the complex conjugate.
    #[must_use]
    #[inline]
    pub const fn conj(self) -> Self {
        Self {
            re: self.re,
            im: -self.im,
        }
    }

    /// Compute the reciprocal (1/z).
    #[must_use]
    #[inline]
    pub fn recip(self) -> Self {
        let norm_sq = self.norm_sqr();
        Self {
            re: self.re / norm_sq,
            im: -self.im / norm_sq,
        }
    }

    /// Compute e^z (complex exponential).
    #[must_use]
    #[inline]
    pub fn exp(self) -> Self {
        let exp_re = libm::exp(self.re);
        Self {
            re: exp_re * libm::cos(self.im),
            im: exp_re * libm::sin(self.im),
        }
    }

    /// Compute the natural logarithm.
    #[must_use]
    #[inline]
    pub fn ln(self) -> Self {
        Self {
            re: libm::log(self.norm()),
            im: self.arg(),
        }
    }

    /// Compute z^n for integer n.
    #[must_use]
    pub fn powi(self, n: i32) -> Self {
        if n == 0 {
            return Self::ONE;
        }
        if n < 0 {
            return self.recip().powi(-n);
        }

        let mut result = Self::ONE;
        let mut base = self;
        let mut exp = n as u32;

        while exp > 0 {
            if exp & 1 == 1 {
                result = result * base;
            }
            base = base * base;
            exp >>= 1;
        }

        result
    }

    /// Compute the square root.
    #[must_use]
    pub fn sqrt(self) -> Self {
        let r = self.norm();
        let theta = self.arg();
        Self::from_polar(libm::sqrt(r), theta / 2.0)
    }

    /// Compute sin(z).
    #[must_use]
    pub fn sin(self) -> Self {
        Self {
            re: libm::sin(self.re) * libm::cosh(self.im),
            im: libm::cos(self.re) * libm::sinh(self.im),
        }
    }

    /// Compute cos(z).
    #[must_use]
    pub fn cos(self) -> Self {
        Self {
            re: libm::cos(self.re) * libm::cosh(self.im),
            im: -libm::sin(self.re) * libm::sinh(self.im),
        }
    }

    /// Compute tanh(z).
    #[must_use]
    pub fn tanh(self) -> Self {
        let two_re = 2.0 * self.re;
        let two_im = 2.0 * self.im;
        let denom = libm::cosh(two_re) + libm::cos(two_im);
        Self {
            re: libm::sinh(two_re) / denom,
            im: libm::sin(two_im) / denom,
        }
    }

    /// Scale by a real number.
    #[must_use]
    #[inline]
    pub fn scale(self, s: f64) -> Self {
        Self {
            re: self.re * s,
            im: self.im * s,
        }
    }
}

impl Add for Complex {
    type Output = Self;

    #[inline]
    fn add(self, rhs: Self) -> Self::Output {
        Self {
            re: self.re + rhs.re,
            im: self.im + rhs.im,
        }
    }
}

impl AddAssign for Complex {
    #[inline]
    fn add_assign(&mut self, rhs: Self) {
        self.re += rhs.re;
        self.im += rhs.im;
    }
}

impl Sub for Complex {
    type Output = Self;

    #[inline]
    fn sub(self, rhs: Self) -> Self::Output {
        Self {
            re: self.re - rhs.re,
            im: self.im - rhs.im,
        }
    }
}

impl SubAssign for Complex {
    #[inline]
    fn sub_assign(&mut self, rhs: Self) {
        self.re -= rhs.re;
        self.im -= rhs.im;
    }
}

impl Mul for Complex {
    type Output = Self;

    #[inline]
    fn mul(self, rhs: Self) -> Self::Output {
        Self {
            re: self.re * rhs.re - self.im * rhs.im,
            im: self.re * rhs.im + self.im * rhs.re,
        }
    }
}

impl MulAssign for Complex {
    #[inline]
    fn mul_assign(&mut self, rhs: Self) {
        *self = *self * rhs;
    }
}

impl Div for Complex {
    type Output = Self;

    #[inline]
    fn div(self, rhs: Self) -> Self::Output {
        let denom = rhs.norm_sqr();
        Self {
            re: (self.re * rhs.re + self.im * rhs.im) / denom,
            im: (self.im * rhs.re - self.re * rhs.im) / denom,
        }
    }
}

impl DivAssign for Complex {
    #[inline]
    fn div_assign(&mut self, rhs: Self) {
        *self = *self / rhs;
    }
}

impl Neg for Complex {
    type Output = Self;

    #[inline]
    fn neg(self) -> Self::Output {
        Self {
            re: -self.re,
            im: -self.im,
        }
    }
}

impl Add<f64> for Complex {
    type Output = Self;

    #[inline]
    fn add(self, rhs: f64) -> Self::Output {
        Self {
            re: self.re + rhs,
            im: self.im,
        }
    }
}

impl Sub<f64> for Complex {
    type Output = Self;

    #[inline]
    fn sub(self, rhs: f64) -> Self::Output {
        Self {
            re: self.re - rhs,
            im: self.im,
        }
    }
}

impl Mul<f64> for Complex {
    type Output = Self;

    #[inline]
    fn mul(self, rhs: f64) -> Self::Output {
        Self {
            re: self.re * rhs,
            im: self.im * rhs,
        }
    }
}

impl Div<f64> for Complex {
    type Output = Self;

    #[inline]
    fn div(self, rhs: f64) -> Self::Output {
        Self {
            re: self.re / rhs,
            im: self.im / rhs,
        }
    }
}

impl From<f64> for Complex {
    fn from(re: f64) -> Self {
        Self::from_real(re)
    }
}

impl From<(f64, f64)> for Complex {
    fn from((re, im): (f64, f64)) -> Self {
        Self::new(re, im)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_basic_arithmetic() {
        let a = Complex::new(1.0, 2.0);
        let b = Complex::new(3.0, 4.0);

        let sum = a + b;
        assert!((sum.re - 4.0).abs() < 1e-10);
        assert!((sum.im - 6.0).abs() < 1e-10);

        let product = a * b;
        // (1+2i)(3+4i) = 3 + 4i + 6i + 8i² = 3 + 10i - 8 = -5 + 10i
        assert!((product.re - (-5.0)).abs() < 1e-10);
        assert!((product.im - 10.0).abs() < 1e-10);
    }

    #[test]
    fn test_polar() {
        let z = Complex::from_polar(2.0, core::f64::consts::PI / 4.0);
        assert!((z.norm() - 2.0).abs() < 1e-10);
        assert!((z.arg() - core::f64::consts::PI / 4.0).abs() < 1e-10);
    }

    #[test]
    fn test_exp() {
        // e^(iπ) = -1
        let z = Complex::new(0.0, core::f64::consts::PI);
        let result = z.exp();
        assert!((result.re - (-1.0)).abs() < 1e-10);
        assert!(result.im.abs() < 1e-10);
    }
}

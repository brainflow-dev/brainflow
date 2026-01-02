//! Vector operations for signal processing.

use alloc::vec::Vec;
use core::ops::{Add, Mul, Sub};

/// A dynamically-sized vector of f64 values.
#[derive(Debug, Clone, PartialEq)]
pub struct Vector {
    data: Vec<f64>,
}

impl Vector {
    /// Create a new vector from a slice.
    #[must_use]
    pub fn from_slice(data: &[f64]) -> Self {
        Self {
            data: data.to_vec(),
        }
    }

    /// Create a vector of zeros.
    #[must_use]
    pub fn zeros(len: usize) -> Self {
        Self {
            data: alloc::vec![0.0; len],
        }
    }

    /// Create a vector of ones.
    #[must_use]
    pub fn ones(len: usize) -> Self {
        Self {
            data: alloc::vec![1.0; len],
        }
    }

    /// Create a vector filled with a constant value.
    #[must_use]
    pub fn filled(len: usize, value: f64) -> Self {
        Self {
            data: alloc::vec![value; len],
        }
    }

    /// Create a linearly spaced vector.
    #[must_use]
    pub fn linspace(start: f64, end: f64, n: usize) -> Self {
        if n == 0 {
            return Self { data: Vec::new() };
        }
        if n == 1 {
            return Self { data: alloc::vec![start] };
        }

        let step = (end - start) / (n - 1) as f64;
        let data: Vec<f64> = (0..n).map(|i| start + step * i as f64).collect();
        Self { data }
    }

    /// Get the length of the vector.
    #[must_use]
    pub fn len(&self) -> usize {
        self.data.len()
    }

    /// Check if the vector is empty.
    #[must_use]
    pub fn is_empty(&self) -> bool {
        self.data.is_empty()
    }

    /// Get a reference to the underlying data.
    #[must_use]
    pub fn as_slice(&self) -> &[f64] {
        &self.data
    }

    /// Get a mutable reference to the underlying data.
    pub fn as_mut_slice(&mut self) -> &mut [f64] {
        &mut self.data
    }

    /// Consume the vector and return the underlying data.
    #[must_use]
    pub fn into_vec(self) -> Vec<f64> {
        self.data
    }

    /// Get an element by index.
    #[must_use]
    pub fn get(&self, index: usize) -> Option<f64> {
        self.data.get(index).copied()
    }

    /// Set an element by index.
    pub fn set(&mut self, index: usize, value: f64) {
        if index < self.data.len() {
            self.data[index] = value;
        }
    }

    /// Compute the dot product with another vector.
    #[must_use]
    pub fn dot(&self, other: &Self) -> f64 {
        self.data
            .iter()
            .zip(other.data.iter())
            .map(|(a, b)| a * b)
            .sum()
    }

    /// Compute the L2 norm (Euclidean length).
    #[must_use]
    pub fn norm(&self) -> f64 {
        libm::sqrt(self.norm_sqr())
    }

    /// Compute the squared L2 norm.
    #[must_use]
    pub fn norm_sqr(&self) -> f64 {
        self.data.iter().map(|x| x * x).sum()
    }

    /// Normalize the vector to unit length.
    #[must_use]
    pub fn normalize(&self) -> Self {
        let n = self.norm();
        if n == 0.0 {
            return self.clone();
        }
        Self {
            data: self.data.iter().map(|x| x / n).collect(),
        }
    }

    /// Compute the sum of all elements.
    #[must_use]
    pub fn sum(&self) -> f64 {
        self.data.iter().sum()
    }

    /// Compute the mean of all elements.
    #[must_use]
    pub fn mean(&self) -> f64 {
        if self.data.is_empty() {
            return 0.0;
        }
        self.sum() / self.data.len() as f64
    }

    /// Find the minimum value.
    #[must_use]
    pub fn min(&self) -> Option<f64> {
        self.data.iter().copied().reduce(f64::min)
    }

    /// Find the maximum value.
    #[must_use]
    pub fn max(&self) -> Option<f64> {
        self.data.iter().copied().reduce(f64::max)
    }

    /// Find the index of the maximum value.
    #[must_use]
    pub fn argmax(&self) -> Option<usize> {
        self.data
            .iter()
            .enumerate()
            .max_by(|(_, a), (_, b)| a.partial_cmp(b).unwrap_or(core::cmp::Ordering::Equal))
            .map(|(i, _)| i)
    }

    /// Find the index of the minimum value.
    #[must_use]
    pub fn argmin(&self) -> Option<usize> {
        self.data
            .iter()
            .enumerate()
            .min_by(|(_, a), (_, b)| a.partial_cmp(b).unwrap_or(core::cmp::Ordering::Equal))
            .map(|(i, _)| i)
    }

    /// Apply a function element-wise.
    #[must_use]
    pub fn map<F>(&self, f: F) -> Self
    where
        F: Fn(f64) -> f64,
    {
        Self {
            data: self.data.iter().map(|&x| f(x)).collect(),
        }
    }

    /// Scale by a constant.
    #[must_use]
    pub fn scale(&self, s: f64) -> Self {
        Self {
            data: self.data.iter().map(|x| x * s).collect(),
        }
    }

    /// Reverse the vector in place.
    pub fn reverse(&mut self) {
        self.data.reverse();
    }

    /// Return a reversed copy.
    #[must_use]
    pub fn reversed(&self) -> Self {
        let mut data = self.data.clone();
        data.reverse();
        Self { data }
    }

    /// Compute element-wise absolute values.
    #[must_use]
    pub fn abs(&self) -> Self {
        self.map(libm::fabs)
    }

    /// Compute element-wise squares.
    #[must_use]
    pub fn square(&self) -> Self {
        self.map(|x| x * x)
    }

    /// Compute element-wise square roots.
    #[must_use]
    pub fn sqrt(&self) -> Self {
        self.map(libm::sqrt)
    }
}

impl core::ops::Index<usize> for Vector {
    type Output = f64;

    fn index(&self, index: usize) -> &Self::Output {
        &self.data[index]
    }
}

impl core::ops::IndexMut<usize> for Vector {
    fn index_mut(&mut self, index: usize) -> &mut Self::Output {
        &mut self.data[index]
    }
}

impl Add for &Vector {
    type Output = Vector;

    fn add(self, rhs: Self) -> Self::Output {
        Vector {
            data: self
                .data
                .iter()
                .zip(rhs.data.iter())
                .map(|(a, b)| a + b)
                .collect(),
        }
    }
}

impl Sub for &Vector {
    type Output = Vector;

    fn sub(self, rhs: Self) -> Self::Output {
        Vector {
            data: self
                .data
                .iter()
                .zip(rhs.data.iter())
                .map(|(a, b)| a - b)
                .collect(),
        }
    }
}

impl Mul<f64> for &Vector {
    type Output = Vector;

    fn mul(self, rhs: f64) -> Self::Output {
        self.scale(rhs)
    }
}

impl From<Vec<f64>> for Vector {
    fn from(data: Vec<f64>) -> Self {
        Self { data }
    }
}

impl From<&[f64]> for Vector {
    fn from(data: &[f64]) -> Self {
        Self::from_slice(data)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_basic_ops() {
        let v = Vector::from_slice(&[1.0, 2.0, 3.0]);
        assert_eq!(v.len(), 3);
        assert!((v.sum() - 6.0).abs() < 1e-10);
        assert!((v.mean() - 2.0).abs() < 1e-10);
    }

    #[test]
    fn test_dot_product() {
        let a = Vector::from_slice(&[1.0, 2.0, 3.0]);
        let b = Vector::from_slice(&[4.0, 5.0, 6.0]);
        // 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
        assert!((a.dot(&b) - 32.0).abs() < 1e-10);
    }

    #[test]
    fn test_linspace() {
        let v = Vector::linspace(0.0, 1.0, 5);
        assert_eq!(v.len(), 5);
        assert!((v[0] - 0.0).abs() < 1e-10);
        assert!((v[4] - 1.0).abs() < 1e-10);
        assert!((v[2] - 0.5).abs() < 1e-10);
    }
}

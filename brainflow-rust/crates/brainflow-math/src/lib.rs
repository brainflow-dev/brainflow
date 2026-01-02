//! Pure Rust mathematical foundations for BrainFlow.
//!
//! This crate provides:
//! - Complex number arithmetic
//! - Matrix and vector operations
//! - Linear algebra (SVD, eigenvalue decomposition)
//! - Statistical functions
//! - Independent Component Analysis (FastICA)

#![no_std]

extern crate alloc;

pub mod complex;
pub mod ica;
pub mod matrix;
pub mod stats;
pub mod vector;

pub mod prelude {
    pub use crate::complex::Complex;
    pub use crate::ica::{fastica, FastIcaParams, FastIcaResult};
    pub use crate::matrix::Matrix;
    pub use crate::stats::{mean, stddev, variance};
    pub use crate::vector::Vector;
}

/// Mathematical constants.
pub mod consts {
    /// Pi (π).
    pub const PI: f64 = core::f64::consts::PI;
    /// Tau (2π).
    pub const TAU: f64 = core::f64::consts::TAU;
    /// Euler's number (e).
    pub const E: f64 = core::f64::consts::E;
    /// Square root of 2.
    pub const SQRT_2: f64 = core::f64::consts::SQRT_2;
}

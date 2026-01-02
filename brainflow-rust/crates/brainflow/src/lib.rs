//! BrainFlow - Pure Rust library for biosignal acquisition and processing.
//!
//! This is a complete rewrite of BrainFlow in pure Rust with no external dependencies.
//!
//! # Overview
//!
//! BrainFlow provides:
//! - **Board Communication**: Connect to 40+ biosensor devices
//! - **Signal Processing**: FFT, digital filters, wavelet transforms
//! - **Machine Learning**: Mindfulness and restfulness classifiers
//!
//! # Quick Start
//!
//! ```ignore
//! use brainflow::prelude::*;
//!
//! // Create a synthetic board for testing
//! let mut board = BoardShim::new(BoardId::Synthetic, BoardParams::new())?;
//!
//! // Prepare and start streaming
//! board.prepare_session()?;
//! board.start_stream(1000)?;
//!
//! // Get data (flattened row-major format)
//! let data = board.get_board_data(Some(100))?;
//!
//! // Process signal data with filters
//! let mut signal: Vec<f64> = vec![1.0, 2.0, 3.0, 4.0, 5.0];
//! DataFilter::perform_lowpass(&mut signal, 250.0, 50.0, 4, FilterType::Butterworth, 0.0);
//!
//! // Clean up
//! board.stop_stream()?;
//! board.release_session()?;
//! ```

#![no_std]

extern crate alloc;

#[cfg(feature = "std")]
extern crate std;

// Re-export sub-crates
pub use brainflow_boards as boards;
pub use brainflow_fft as fft;
pub use brainflow_filter as filter;
pub use brainflow_io as io;
pub use brainflow_math as math;
pub use brainflow_ml as ml;
pub use brainflow_sys as sys;
pub use brainflow_wavelet as wavelet;

// Re-export commonly used types
pub use brainflow_boards::{BoardId, BoardParams, Preset};
pub use brainflow_fft::WindowType;
pub use brainflow_filter::{DetrendType, FilterType, NoiseType};
pub use brainflow_ml::{ClassifierType, Metric};
pub use brainflow_sys::error::{Error, ErrorCode, Result};
pub use brainflow_wavelet::Wavelet;

pub mod board_shim;
pub mod data_filter;
pub mod ml_model;

pub use board_shim::BoardShim;
pub use data_filter::DataFilter;
pub use ml_model::MLModel;

/// Prelude module for convenient imports.
pub mod prelude {
    pub use super::board_shim::BoardShim;
    pub use super::data_filter::DataFilter;
    pub use super::ml_model::MLModel;

    pub use super::{
        BoardId, BoardParams, ClassifierType, DetrendType, Error, ErrorCode,
        FilterType, Metric, NoiseType, Preset, Result, Wavelet, WindowType,
    };
}

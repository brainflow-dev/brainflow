//! Pure Rust ML inference for BrainFlow.
//!
//! This crate provides:
//! - ML model trait and common types
//! - Built-in mindfulness/restfulness classifiers
//! - Custom model loading support

#![no_std]

extern crate alloc;

use alloc::string::String;
use alloc::vec::Vec;

pub mod classifier;
pub mod mindfulness;

pub use classifier::{Classifier, ClassifierParams};
pub use mindfulness::{MindfulnessClassifier, RestfulnessClassifier};

/// Supported metrics for classification.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
#[repr(i32)]
pub enum Metric {
    /// Mindfulness metric.
    #[default]
    Mindfulness = 0,
    /// Restfulness metric.
    Restfulness = 1,
    /// User-defined metric.
    UserDefined = 2,
}

/// Classifier types.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
#[repr(i32)]
pub enum ClassifierType {
    /// Default built-in classifier.
    #[default]
    Default = 0,
    /// Dynamic library classifier (not supported in pure Rust).
    DynLib = 1,
    /// Custom model file.
    Custom = 2,
}

/// ML model error codes.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum MlError {
    /// Model not prepared.
    NotPrepared,
    /// Another model is prepared.
    AnotherPrepared,
    /// Invalid input.
    InvalidInput,
    /// Unsupported metric/classifier combination.
    Unsupported,
    /// Model file not found.
    FileNotFound,
    /// Inference failed.
    InferenceFailed,
}

/// Result type for ML operations.
pub type MlResult<T> = Result<T, MlError>;

/// Perform ML prediction.
///
/// This is the main interface matching BrainFlow's MLModel.
pub fn predict(
    data: &[f64],
    metric: Metric,
    classifier: ClassifierType,
    model_file: Option<&str>,
) -> MlResult<f64> {
    match (metric, classifier) {
        (Metric::Mindfulness, ClassifierType::Default) => {
            let mut clf = MindfulnessClassifier::new();
            clf.prepare()?;
            let result = clf.predict(data)?;
            clf.release()?;
            Ok(result)
        }
        (Metric::Restfulness, ClassifierType::Default) => {
            let mut clf = RestfulnessClassifier::new();
            clf.prepare()?;
            let result = clf.predict(data)?;
            clf.release()?;
            Ok(result)
        }
        _ => Err(MlError::Unsupported),
    }
}

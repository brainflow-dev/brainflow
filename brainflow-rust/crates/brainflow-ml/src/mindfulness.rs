//! Mindfulness and restfulness classifiers.

use crate::classifier::{Classifier, features};
use crate::{MlError, MlResult};

/// Mindfulness classifier.
///
/// Uses alpha/theta ratio and other EEG features to estimate mindfulness.
#[derive(Debug, Default)]
pub struct MindfulnessClassifier {
    prepared: bool,
}

impl MindfulnessClassifier {
    /// Create a new mindfulness classifier.
    pub fn new() -> Self {
        Self { prepared: false }
    }

    /// Compute mindfulness score from features.
    ///
    /// This is a simplified implementation. The actual BrainFlow
    /// implementation uses a trained model.
    fn compute_score(&self, alpha_theta_ratio: f64, beta_alpha_ratio: f64) -> f64 {
        // Higher alpha/theta ratio indicates more relaxed, focused state
        // Lower beta/alpha ratio indicates less mental activity

        // Normalize ratios to [0, 1] range using sigmoid-like function
        let alpha_score = 1.0 / (1.0 + libm::exp(-0.5 * (alpha_theta_ratio - 1.5)));
        let beta_score = 1.0 / (1.0 + libm::exp(0.5 * (beta_alpha_ratio - 1.0)));

        // Combine scores
        let score = 0.6 * alpha_score + 0.4 * beta_score;

        // Clamp to [0, 1]
        score.max(0.0).min(1.0)
    }
}

impl Classifier for MindfulnessClassifier {
    fn prepare(&mut self) -> MlResult<()> {
        self.prepared = true;
        Ok(())
    }

    fn predict(&self, data: &[f64]) -> MlResult<f64> {
        if !self.prepared {
            return Err(MlError::NotPrepared);
        }

        if data.is_empty() {
            return Err(MlError::InvalidInput);
        }

        // Assume data is already PSD with frequencies
        // In a real implementation, we'd compute PSD from raw EEG

        // For this simplified version, compute features directly
        let mean_val = brainflow_math::stats::mean(data);
        let std_val = brainflow_math::stats::stddev(data);

        // Use statistical features as proxy for band power ratios
        let alpha_theta_ratio = if std_val > 1e-10 { mean_val / std_val } else { 1.0 };
        let beta_alpha_ratio = std_val / (mean_val.abs() + 1e-10);

        Ok(self.compute_score(alpha_theta_ratio, beta_alpha_ratio))
    }

    fn release(&mut self) -> MlResult<()> {
        self.prepared = false;
        Ok(())
    }

    fn is_prepared(&self) -> bool {
        self.prepared
    }
}

/// Restfulness classifier.
///
/// Inverse of mindfulness - higher values indicate more restful state.
#[derive(Debug, Default)]
pub struct RestfulnessClassifier {
    inner: MindfulnessClassifier,
}

impl RestfulnessClassifier {
    /// Create a new restfulness classifier.
    pub fn new() -> Self {
        Self {
            inner: MindfulnessClassifier::new(),
        }
    }
}

impl Classifier for RestfulnessClassifier {
    fn prepare(&mut self) -> MlResult<()> {
        self.inner.prepare()
    }

    fn predict(&self, data: &[f64]) -> MlResult<f64> {
        // Restfulness is inverse of mindfulness
        let mindfulness = self.inner.predict(data)?;
        Ok(1.0 - mindfulness)
    }

    fn release(&mut self) -> MlResult<()> {
        self.inner.release()
    }

    fn is_prepared(&self) -> bool {
        self.inner.is_prepared()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_mindfulness_classifier() {
        let mut clf = MindfulnessClassifier::new();

        assert!(!clf.is_prepared());

        clf.prepare().unwrap();
        assert!(clf.is_prepared());

        let data = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0];
        let score = clf.predict(&data).unwrap();

        assert!(score >= 0.0 && score <= 1.0);

        clf.release().unwrap();
        assert!(!clf.is_prepared());
    }

    #[test]
    fn test_restfulness_classifier() {
        let mut clf = RestfulnessClassifier::new();
        clf.prepare().unwrap();

        let data = [1.0, 2.0, 3.0, 4.0, 5.0];
        let score = clf.predict(&data).unwrap();

        assert!(score >= 0.0 && score <= 1.0);

        clf.release().unwrap();
    }

    #[test]
    fn test_restfulness_inverse() {
        let mut mind_clf = MindfulnessClassifier::new();
        let mut rest_clf = RestfulnessClassifier::new();

        mind_clf.prepare().unwrap();
        rest_clf.prepare().unwrap();

        let data = [1.0, 2.0, 3.0, 4.0, 5.0];

        let mindfulness = mind_clf.predict(&data).unwrap();
        let restfulness = rest_clf.predict(&data).unwrap();

        assert!((mindfulness + restfulness - 1.0).abs() < 1e-10);
    }
}

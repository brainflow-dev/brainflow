//! MLModel - Machine learning utilities.

use brainflow_ml::{ClassifierType, Metric, MlError, MlResult};

/// Machine learning model interface matching the C++ MLModel API.
pub struct MLModel {
    metric: Metric,
    classifier: ClassifierType,
    prepared: bool,
}

impl MLModel {
    /// Create a new MLModel.
    pub fn new(metric: Metric, classifier: ClassifierType) -> Self {
        Self {
            metric,
            classifier,
            prepared: false,
        }
    }

    /// Prepare the model.
    pub fn prepare(&mut self) -> MlResult<()> {
        if self.prepared {
            return Err(MlError::AnotherPrepared);
        }
        self.prepared = true;
        Ok(())
    }

    /// Run prediction on input data.
    pub fn predict(&self, data: &[f64]) -> MlResult<f64> {
        if !self.prepared {
            return Err(MlError::NotPrepared);
        }
        brainflow_ml::predict(data, self.metric, self.classifier, None)
    }

    /// Release the model.
    pub fn release(&mut self) -> MlResult<()> {
        self.prepared = false;
        Ok(())
    }

    /// Check if the model is prepared.
    pub fn is_prepared(&self) -> bool {
        self.prepared
    }
}

impl Drop for MLModel {
    fn drop(&mut self) {
        let _ = self.release();
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_ml_model_mindfulness() {
        let mut model = MLModel::new(Metric::Mindfulness, ClassifierType::Default);

        assert!(!model.is_prepared());

        model.prepare().unwrap();
        assert!(model.is_prepared());

        let data = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0];
        let score = model.predict(&data).unwrap();

        assert!(score >= 0.0 && score <= 1.0);

        model.release().unwrap();
        assert!(!model.is_prepared());
    }

    #[test]
    fn test_ml_model_restfulness() {
        let mut model = MLModel::new(Metric::Restfulness, ClassifierType::Default);
        model.prepare().unwrap();

        let data = [1.0, 2.0, 3.0, 4.0, 5.0];
        let score = model.predict(&data).unwrap();

        assert!(score >= 0.0 && score <= 1.0);
    }
}

use getset::Getters;
use serde::{Deserialize, Serialize};

/// Inputs parameters for MlModel.
#[derive(Debug, Default, PartialEq, Eq, PartialOrd, Ord, Serialize, Deserialize, Getters)]
#[getset(get = "pub", set = "pub")]
pub struct BrainFlowModelParams {
    metric: usize,
    classifier: usize,
    file: String,
    other_info: String,
}

/// Builder for [BrainFlowModelParams].
#[derive(Default)]
pub struct BrainFlowModelParamsBuilder {
    params: BrainFlowModelParams,
}

impl BrainFlowModelParamsBuilder {
    /// Create a new builder.
    pub fn new() -> Self {
        Default::default()
    }

    /// Metric to calculate.
    pub fn metric(mut self, metric: usize) -> Self {
        self.params.metric = metric;
        self
    }

    /// Classifier to use.
    pub fn classifier(mut self, classifier: usize) -> Self {
        self.params.classifier = classifier;
        self
    }

    /// File to load model.
    pub fn file<S: AsRef<str>>(mut self, file: S) -> Self {
        self.params.file = file.as_ref().to_string();
        self
    }

    /// Other info.
    pub fn other_info<S: AsRef<str>>(mut self, other_info: S) -> Self {
        self.params.other_info = other_info.as_ref().to_string();
        self
    }

    /// Build BrainFlowModelParams with the given options.
    pub fn build(self) -> BrainFlowModelParams {
        self.params
    }
}

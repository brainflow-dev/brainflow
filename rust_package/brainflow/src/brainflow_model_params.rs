use getset::Getters;
use serde::{ser::SerializeStruct, Serialize};

use crate::{BrainFlowClassifiers, BrainFlowMetrics};

/// Inputs parameters for MlModel.
#[derive(Debug, Getters)]
#[getset(get = "pub", set = "pub")]
pub struct BrainFlowModelParams {
    metric: BrainFlowMetrics,
    classifier: BrainFlowClassifiers,
    file: String,
    other_info: String,
    output_name: String,
    max_array_size: usize,
}

impl Serialize for BrainFlowModelParams {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        let mut state = serializer.serialize_struct("BrainFlowModelParams", 4)?;
        state.serialize_field("metric", &(self.metric as usize))?;
        state.serialize_field("classifier", &(self.classifier as usize))?;
        state.serialize_field("file", &self.file.to_string())?;
        state.serialize_field("other_info", &self.other_info.to_string())?;
        state.serialize_field("output_name", &self.output_name.to_string())?;
        state.serialize_field("max_array_size", &self.max_array_size)?;
        state.end()
    }
}

impl Default for BrainFlowModelParams {
    fn default() -> Self {
        Self {
            metric: BrainFlowMetrics::Mindfulness,
            classifier: BrainFlowClassifiers::DefaultClassifier,
            file: Default::default(),
            other_info: Default::default(),
            output_name: "".to_string(),
            max_array_size: 8192,
        }
    }
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
    pub fn metric(mut self, metric: BrainFlowMetrics) -> Self {
        self.params.metric = metric;
        self
    }

    /// Classifier to use.
    pub fn classifier(mut self, classifier: BrainFlowClassifiers) -> Self {
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

    /// Output name.
    pub fn output_name<S: AsRef<str>>(mut self, output_name: S) -> Self {
        self.params.output_name = output_name.as_ref().to_string();
        self
    }

    /// Max array size.
    pub fn max_array_size(mut self, max_array_size: usize) -> Self {
        self.params.max_array_size = max_array_size;
        self
    }

    /// Build BrainFlowModelParams with the given options.
    pub fn build(self) -> BrainFlowModelParams {
        self.params
    }
}

use getset::Getters;
use serde::{Deserialize, Serialize};

#[derive(Debug, Default, PartialEq, Eq, PartialOrd, Ord, Serialize, Deserialize, Getters)]
#[getset(get = "pub", set = "pub")]
pub struct BrainFlowModelParams {
    metric: String,
    classifier: String,
    file: String,
    other_info: String,
}

#[derive(Default)]
pub struct BrainFlowModelParamsBuilder {
    params: BrainFlowModelParams,
}

impl BrainFlowModelParamsBuilder {
    pub fn new() -> Self {
        Default::default()
    }

    pub fn metric<S: AsRef<str>>(mut self, metric: S) -> Self {
        self.params.metric = metric.as_ref().to_string();
        self
    }

    pub fn classifier<S: AsRef<str>>(mut self, classifier: S) -> Self {
        self.params.classifier = classifier.as_ref().to_string();
        self
    }

    pub fn file<S: AsRef<str>>(mut self, file: S) -> Self {
        self.params.file = file.as_ref().to_string();
        self
    }

    pub fn other_info<S: AsRef<str>>(mut self, other_info: S) -> Self {
        self.params.other_info = other_info.as_ref().to_string();
        self
    }

    pub fn build(self) -> BrainFlowModelParams {
        self.params
    }
}

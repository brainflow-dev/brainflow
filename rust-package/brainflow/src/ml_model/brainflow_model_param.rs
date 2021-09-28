use getset::Getters;
use serde::{Deserialize, Serialize};

#[derive(Debug, Default, PartialEq, Eq, PartialOrd, Ord, Serialize, Deserialize, Getters)]
#[getset(get = "pub", set = "pub")]
pub struct BrainFlowModelParams {
    metric: usize,
    classifier: usize,
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

    pub fn metric(mut self, metric: usize) -> Self {
        self.params.metric = metric;
        self
    }

    pub fn classifier(mut self, classifier: usize) -> Self {
        self.params.classifier = classifier;
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

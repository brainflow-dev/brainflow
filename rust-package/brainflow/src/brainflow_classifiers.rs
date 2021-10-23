#[derive(Debug, Copy, Clone, Hash, PartialEq, Eq, FromPrimitive)]
pub enum BrainFlowClassifiers {
    Regression = 0,
    Knn = 1,
    Svm = 2,
    Lda = 3,
}
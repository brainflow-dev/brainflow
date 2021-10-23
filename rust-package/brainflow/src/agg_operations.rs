#[derive(Debug, Copy, Clone, Hash, PartialEq, Eq, FromPrimitive)]
pub enum AggOperations {
    Mean = 0,
    Median = 1,
    Each = 2,
}
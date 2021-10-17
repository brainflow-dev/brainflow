#[derive(Debug, Copy, Clone, Hash, PartialEq, Eq, FromPrimitive)]
pub enum DetrendOperations {
    None = 0,
    Constant = 1,
    Linear = 2,
}
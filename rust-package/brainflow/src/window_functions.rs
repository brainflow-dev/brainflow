#[derive(Debug, Copy, Clone, Hash, PartialEq, Eq, FromPrimitive)]
pub enum WindowFunctions {
    NoWindow = 0,
    Hanning = 1,
    Hamming = 2,
    BlackmanHarris = 3,
}
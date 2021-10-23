#[derive(Debug, Copy, Clone, Hash, PartialEq, Eq, FromPrimitive)]
pub enum FilterTypes {
    Butterworth = 0,
    ChebyshevType1 = 1,
    Bessel = 2,
}
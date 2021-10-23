#[derive(Debug, Copy, Clone, Hash, PartialEq, Eq, FromPrimitive)]
pub enum IpProtocolType {
    None = 0,
    Udp = 1,
    Tcp = 2,
}
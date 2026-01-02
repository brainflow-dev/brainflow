//! OpenBCI board implementations.
//!
//! This module contains implementations for OpenBCI boards:
//! - Cyton (8-channel, serial)
//! - CytonDaisy (16-channel, serial)
//! - FreeEEG32 (32-channel, serial)
//! - Ganglion (4-channel, BLE)
//! - WiFi variants

mod cyton;
mod freeeeg;
mod packet;

pub use cyton::{CytonBoard, CytonDaisyBoard};
pub use freeeeg::FreeEeg32Board;
pub use packet::{OpenBciPacket, PacketType};

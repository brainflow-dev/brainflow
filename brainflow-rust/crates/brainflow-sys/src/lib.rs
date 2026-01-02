//! Low-level OS abstractions for BrainFlow.
//!
//! This crate provides platform-specific implementations for:
//! - Time and timestamps
//! - Thread synchronization primitives
//! - Dynamic library loading
//! - Environment and system information

#![no_std]

extern crate alloc;

pub mod error;
pub mod sync;
pub mod time;

#[cfg(feature = "std")]
extern crate std;

/// Re-export commonly used types.
pub mod prelude {
    pub use crate::error::{Error, Result};
    pub use crate::sync::SpinLock;
    pub use crate::time::Timestamp;
}

//! Time and timestamp utilities.

/// A timestamp in microseconds since an arbitrary epoch.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct Timestamp(pub u64);

impl Timestamp {
    /// Create a timestamp from microseconds.
    #[must_use]
    pub const fn from_micros(micros: u64) -> Self {
        Self(micros)
    }

    /// Create a timestamp from milliseconds.
    #[must_use]
    pub const fn from_millis(millis: u64) -> Self {
        Self(millis * 1000)
    }

    /// Create a timestamp from seconds.
    #[must_use]
    pub const fn from_secs(secs: u64) -> Self {
        Self(secs * 1_000_000)
    }

    /// Create a timestamp from seconds as f64.
    #[must_use]
    pub fn from_secs_f64(secs: f64) -> Self {
        Self((secs * 1_000_000.0) as u64)
    }

    /// Get the timestamp in microseconds.
    #[must_use]
    pub const fn as_micros(self) -> u64 {
        self.0
    }

    /// Get the timestamp in milliseconds.
    #[must_use]
    pub const fn as_millis(self) -> u64 {
        self.0 / 1000
    }

    /// Get the timestamp in seconds.
    #[must_use]
    pub const fn as_secs(self) -> u64 {
        self.0 / 1_000_000
    }

    /// Get the timestamp in seconds as f64.
    #[must_use]
    pub fn as_secs_f64(self) -> f64 {
        self.0 as f64 / 1_000_000.0
    }

    /// Calculate duration between two timestamps.
    #[must_use]
    pub const fn duration_since(self, earlier: Self) -> u64 {
        self.0.saturating_sub(earlier.0)
    }
}

impl core::ops::Add<u64> for Timestamp {
    type Output = Self;

    fn add(self, rhs: u64) -> Self::Output {
        Self(self.0 + rhs)
    }
}

impl core::ops::Sub<u64> for Timestamp {
    type Output = Self;

    fn sub(self, rhs: u64) -> Self::Output {
        Self(self.0 - rhs)
    }
}

impl core::ops::Sub for Timestamp {
    type Output = u64;

    fn sub(self, rhs: Self) -> Self::Output {
        self.0.saturating_sub(rhs.0)
    }
}

/// Get the current timestamp.
///
/// This is platform-specific and requires std.
#[cfg(feature = "std")]
pub fn now() -> Timestamp {
    use std::time::{SystemTime, UNIX_EPOCH};

    let duration = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap_or_default();

    Timestamp::from_micros(duration.as_micros() as u64)
}

/// High-resolution monotonic clock for timing.
#[derive(Debug, Clone, Copy)]
pub struct Instant {
    ticks: u64,
}

impl Instant {
    /// Create an instant representing "now" (placeholder for no_std).
    #[must_use]
    pub const fn now_placeholder() -> Self {
        Self { ticks: 0 }
    }

    /// Elapsed time in microseconds since this instant.
    #[must_use]
    pub const fn elapsed_micros(&self, now_ticks: u64) -> u64 {
        now_ticks.saturating_sub(self.ticks)
    }
}

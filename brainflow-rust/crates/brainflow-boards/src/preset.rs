//! Data presets for board channels.

/// Data preset types matching BrainFlow constants.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default)]
#[repr(i32)]
pub enum Preset {
    /// Default preset (main EEG/EMG channels).
    #[default]
    Default = 0,
    /// Auxiliary preset (accelerometer, gyroscope, etc.).
    Auxiliary = 1,
    /// Ancillary preset (additional sensors).
    Ancillary = 2,
}

impl Preset {
    /// Convert from raw i32 value.
    pub fn from_raw(value: i32) -> Option<Self> {
        match value {
            0 => Some(Self::Default),
            1 => Some(Self::Auxiliary),
            2 => Some(Self::Ancillary),
            _ => None,
        }
    }

    /// Convert to raw i32 value.
    pub fn to_raw(self) -> i32 {
        self as i32
    }

    /// Get all preset variants.
    pub fn all() -> &'static [Preset] {
        &[Self::Default, Self::Auxiliary, Self::Ancillary]
    }
}

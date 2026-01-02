//! Board trait and common types.

use alloc::string::String;
use alloc::vec::Vec;
use brainflow_sys::error::Result;

use crate::preset::Preset;

/// Board state.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum BoardState {
    /// Board is not initialized.
    #[default]
    NotReady,
    /// Session is prepared but not streaming.
    Ready,
    /// Board is actively streaming data.
    Streaming,
}

/// Board description with channel information.
#[derive(Debug, Clone, Default)]
pub struct BoardDescription {
    /// Sampling rate in Hz.
    pub sampling_rate: u32,
    /// Number of channels.
    pub num_channels: usize,
    /// EEG channel indices.
    pub eeg_channels: Vec<usize>,
    /// EMG channel indices.
    pub emg_channels: Vec<usize>,
    /// ECG channel indices.
    pub ecg_channels: Vec<usize>,
    /// EOG channel indices.
    pub eog_channels: Vec<usize>,
    /// PPG channel indices.
    pub ppg_channels: Vec<usize>,
    /// EDA channel indices.
    pub eda_channels: Vec<usize>,
    /// Accelerometer channel indices.
    pub accel_channels: Vec<usize>,
    /// Gyroscope channel indices.
    pub gyro_channels: Vec<usize>,
    /// Temperature channel indices.
    pub temperature_channels: Vec<usize>,
    /// Battery channel index.
    pub battery_channel: Option<usize>,
    /// Timestamp channel index.
    pub timestamp_channel: Option<usize>,
    /// Marker channel index.
    pub marker_channel: Option<usize>,
    /// Package number channel index.
    pub package_num_channel: Option<usize>,
    /// Device name.
    pub name: String,
}

/// Trait for board implementations.
pub trait Board: Send + Sync {
    /// Get the board's current state.
    fn state(&self) -> BoardState;

    /// Get the board description for a preset.
    fn description(&self, preset: Preset) -> &BoardDescription;

    /// Prepare the session (initialize hardware connection).
    fn prepare_session(&mut self) -> Result<()>;

    /// Release the session (cleanup and disconnect).
    fn release_session(&mut self) -> Result<()>;

    /// Start data streaming.
    ///
    /// # Arguments
    /// * `buffer_size` - Size of the ring buffer in samples.
    fn start_stream(&mut self, buffer_size: usize) -> Result<()>;

    /// Stop data streaming.
    fn stop_stream(&mut self) -> Result<()>;

    /// Get current data from the buffer without removing it.
    ///
    /// Returns data as a flattened array (num_channels × num_samples).
    fn get_current_board_data(&self, num_samples: usize, preset: Preset) -> Result<Vec<f64>>;

    /// Get data from the buffer and remove it.
    ///
    /// Returns data as a flattened array (num_channels × num_samples).
    fn get_board_data(&self, num_samples: Option<usize>, preset: Preset) -> Result<Vec<f64>>;

    /// Get the number of samples currently in the buffer.
    fn get_board_data_count(&self, preset: Preset) -> Result<usize>;

    /// Insert a marker into the data stream.
    fn insert_marker(&mut self, value: f64, preset: Preset) -> Result<()>;

    /// Send a configuration string to the board.
    fn config_board(&mut self, config: &str) -> Result<String>;

    /// Check if the board is prepared.
    fn is_prepared(&self) -> bool {
        matches!(self.state(), BoardState::Ready | BoardState::Streaming)
    }

    /// Check if the board is streaming.
    fn is_streaming(&self) -> bool {
        self.state() == BoardState::Streaming
    }
}

/// Helper to reshape data from buffer format to row-major matrix.
///
/// Buffer format: sample1_ch1, sample1_ch2, ..., sample2_ch1, ...
/// Row-major format: all of ch1, all of ch2, ...
pub fn reshape_to_row_major(data: &[f64], num_channels: usize) -> Vec<f64> {
    if data.is_empty() || num_channels == 0 {
        return Vec::new();
    }

    let num_samples = data.len() / num_channels;
    let mut result = Vec::with_capacity(data.len());

    for channel in 0..num_channels {
        for sample in 0..num_samples {
            result.push(data[sample * num_channels + channel]);
        }
    }

    result
}

/// Helper to reshape data from row-major matrix to buffer format.
pub fn reshape_from_row_major(data: &[f64], num_channels: usize) -> Vec<f64> {
    if data.is_empty() || num_channels == 0 {
        return Vec::new();
    }

    let num_samples = data.len() / num_channels;
    let mut result = Vec::with_capacity(data.len());

    for sample in 0..num_samples {
        for channel in 0..num_channels {
            result.push(data[channel * num_samples + sample]);
        }
    }

    result
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::vec;

    #[test]
    fn test_reshape_to_row_major() {
        // Buffer: [s0_ch0, s0_ch1, s1_ch0, s1_ch1, s2_ch0, s2_ch1]
        let buffer = vec![1.0, 2.0, 3.0, 4.0, 5.0, 6.0];
        // Row-major: [s0_ch0, s1_ch0, s2_ch0, s0_ch1, s1_ch1, s2_ch1]
        let expected = vec![1.0, 3.0, 5.0, 2.0, 4.0, 6.0];

        let result = reshape_to_row_major(&buffer, 2);
        assert_eq!(result, expected);
    }

    #[test]
    fn test_reshape_roundtrip() {
        let original = vec![1.0, 2.0, 3.0, 4.0, 5.0, 6.0];
        let row_major = reshape_to_row_major(&original, 2);
        let back = reshape_from_row_major(&row_major, 2);
        assert_eq!(back, original);
    }
}

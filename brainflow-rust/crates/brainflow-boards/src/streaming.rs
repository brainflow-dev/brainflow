//! Streaming board implementation.
//!
//! Receives BrainFlow data over a network connection from another board.

use alloc::string::String;
use alloc::vec;
use alloc::vec::Vec;

use brainflow_sys::error::{Error, ErrorCode, Result};
use brainflow_sys::sync::Mutex;

use crate::board::{Board, BoardDescription, BoardState};
use crate::preset::Preset;
use crate::BoardId;

/// Streaming board that receives data over the network.
pub struct StreamingBoard {
    state: BoardState,
    ip_address: String,
    port: u16,
    master_board: BoardId,
    descriptions: [BoardDescription; 3],
    buffer: Mutex<Vec<f64>>,
    num_cols: usize,
}

impl StreamingBoard {
    /// Create a new streaming board.
    pub fn new(ip_address: String, port: u16, master_board: BoardId) -> Self {
        let desc = Self::get_master_description(master_board);
        let num_cols = desc.num_channels + 3; // channels + timestamp + marker + package

        Self {
            state: BoardState::NotReady,
            ip_address,
            port,
            master_board,
            descriptions: [desc.clone(), desc.clone(), desc],
            buffer: Mutex::new(Vec::new()),
            num_cols,
        }
    }

    /// Get board description from master board ID.
    fn get_master_description(board_id: BoardId) -> BoardDescription {
        match board_id {
            BoardId::Synthetic => BoardDescription {
                name: String::from("Synthetic Stream"),
                sampling_rate: 250,
                num_channels: 16,
                eeg_channels: vec![1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16],
                emg_channels: vec![],
                ecg_channels: vec![],
                eog_channels: vec![],
                ppg_channels: vec![],
                eda_channels: vec![],
                accel_channels: vec![],
                gyro_channels: vec![],
                temperature_channels: vec![],
                battery_channel: None,
                timestamp_channel: Some(17),
                marker_channel: Some(18),
                package_num_channel: Some(0),
            },
            BoardId::Cyton => BoardDescription {
                name: String::from("Cyton Stream"),
                sampling_rate: 250,
                num_channels: 8,
                eeg_channels: vec![1, 2, 3, 4, 5, 6, 7, 8],
                emg_channels: vec![],
                ecg_channels: vec![],
                eog_channels: vec![],
                ppg_channels: vec![],
                eda_channels: vec![],
                accel_channels: vec![9, 10, 11],
                gyro_channels: vec![],
                temperature_channels: vec![],
                battery_channel: None,
                timestamp_channel: Some(12),
                marker_channel: Some(13),
                package_num_channel: Some(0),
            },
            BoardId::Ganglion => BoardDescription {
                name: String::from("Ganglion Stream"),
                sampling_rate: 200,
                num_channels: 4,
                eeg_channels: vec![1, 2, 3, 4],
                emg_channels: vec![],
                ecg_channels: vec![],
                eog_channels: vec![],
                ppg_channels: vec![],
                eda_channels: vec![],
                accel_channels: vec![5, 6, 7],
                gyro_channels: vec![],
                temperature_channels: vec![],
                battery_channel: None,
                timestamp_channel: Some(8),
                marker_channel: Some(9),
                package_num_channel: Some(0),
            },
            _ => BoardDescription {
                name: String::from("Stream"),
                sampling_rate: 250,
                num_channels: 8,
                eeg_channels: vec![1, 2, 3, 4, 5, 6, 7, 8],
                emg_channels: vec![],
                ecg_channels: vec![],
                eog_channels: vec![],
                ppg_channels: vec![],
                eda_channels: vec![],
                accel_channels: vec![],
                gyro_channels: vec![],
                temperature_channels: vec![],
                battery_channel: None,
                timestamp_channel: Some(9),
                marker_channel: Some(10),
                package_num_channel: Some(0),
            },
        }
    }

    /// Get preset index.
    fn preset_index(preset: Preset) -> usize {
        match preset {
            Preset::Default => 0,
            Preset::Auxiliary => 1,
            Preset::Ancillary => 2,
        }
    }

    /// Parse incoming data packet.
    #[allow(dead_code)]
    fn parse_packet(data: &[u8]) -> Vec<f64> {
        // BrainFlow streaming format: each value is 8 bytes (f64, little-endian)
        let mut values = Vec::new();

        for chunk in data.chunks(8) {
            if chunk.len() == 8 {
                let bytes: [u8; 8] = chunk.try_into().unwrap_or([0; 8]);
                values.push(f64::from_le_bytes(bytes));
            }
        }

        values
    }

    /// Add data to the buffer (called from network receive).
    pub fn push_data(&self, data: &[f64]) {
        let mut buffer = self.buffer.lock();
        buffer.extend_from_slice(data);
    }

    /// Get the IP address.
    #[allow(dead_code)]
    pub fn ip_address(&self) -> &str {
        &self.ip_address
    }

    /// Get the port.
    #[allow(dead_code)]
    pub fn port(&self) -> u16 {
        self.port
    }

    /// Get the master board ID.
    #[allow(dead_code)]
    pub fn master_board(&self) -> BoardId {
        self.master_board
    }
}

impl Board for StreamingBoard {
    fn state(&self) -> BoardState {
        self.state
    }

    fn description(&self, preset: Preset) -> &BoardDescription {
        &self.descriptions[Self::preset_index(preset)]
    }

    fn prepare_session(&mut self) -> Result<()> {
        if self.state != BoardState::NotReady {
            return Err(Error::new(ErrorCode::AnotherBoardCreated));
        }

        // In a real implementation, we would set up the UDP socket here
        // For no_std, socket I/O would need to be provided externally

        self.state = BoardState::Ready;
        Ok(())
    }

    fn release_session(&mut self) -> Result<()> {
        if self.state == BoardState::NotReady {
            return Err(Error::new(ErrorCode::BoardNotCreated));
        }

        self.state = BoardState::NotReady;
        *self.buffer.lock() = Vec::new();
        Ok(())
    }

    fn start_stream(&mut self, _buffer_size: usize) -> Result<()> {
        if self.state != BoardState::Ready {
            return Err(Error::new(ErrorCode::BoardNotReady));
        }

        self.state = BoardState::Streaming;

        // In a real implementation, we would start receiving UDP packets here
        // and spawning a thread to handle incoming data

        Ok(())
    }

    fn stop_stream(&mut self) -> Result<()> {
        if self.state != BoardState::Streaming {
            return Err(Error::new(ErrorCode::StreamThreadNotRunning));
        }

        self.state = BoardState::Ready;
        Ok(())
    }

    fn get_current_board_data(&self, num_samples: usize, _preset: Preset) -> Result<Vec<f64>> {
        let buffer = self.buffer.lock();
        let total_samples = buffer.len() / self.num_cols;
        let samples_to_get = num_samples.min(total_samples);
        let start_idx = total_samples.saturating_sub(samples_to_get);

        let start = start_idx * self.num_cols;
        let end = (start_idx + samples_to_get) * self.num_cols;

        Ok(buffer[start..end].to_vec())
    }

    fn get_board_data(&self, num_samples: Option<usize>, _preset: Preset) -> Result<Vec<f64>> {
        let mut buffer = self.buffer.lock();
        let total_samples = buffer.len() / self.num_cols;
        let samples_to_get = num_samples.unwrap_or(total_samples).min(total_samples);

        let end = samples_to_get * self.num_cols;
        let result = buffer[..end].to_vec();

        buffer.drain(..end);

        Ok(result)
    }

    fn get_board_data_count(&self, _preset: Preset) -> Result<usize> {
        let buffer = self.buffer.lock();
        Ok(buffer.len() / self.num_cols)
    }

    fn insert_marker(&mut self, value: f64, _preset: Preset) -> Result<()> {
        // Insert marker into the latest sample
        let mut buffer = self.buffer.lock();

        if buffer.len() >= self.num_cols {
            if let Some(marker_ch) = self.descriptions[0].marker_channel {
                let marker_idx = buffer.len() - self.num_cols + marker_ch;
                if marker_idx < buffer.len() {
                    buffer[marker_idx] = value;
                }
            }
        }

        Ok(())
    }

    fn config_board(&mut self, _config: &str) -> Result<String> {
        // Streaming board doesn't support configuration
        Ok(String::new())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_streaming_board_new() {
        let board = StreamingBoard::new(
            String::from("127.0.0.1"),
            5000,
            BoardId::Synthetic,
        );
        assert_eq!(board.state(), BoardState::NotReady);
    }

    #[test]
    fn test_parse_packet() {
        let value = 42.5_f64;
        let bytes = value.to_le_bytes();
        let parsed = StreamingBoard::parse_packet(&bytes);
        assert_eq!(parsed.len(), 1);
        assert!((parsed[0] - 42.5).abs() < 1e-10);
    }

    #[test]
    fn test_push_data() {
        let board = StreamingBoard::new(
            String::from("127.0.0.1"),
            5000,
            BoardId::Synthetic,
        );
        board.push_data(&[1.0, 2.0, 3.0]);

        let buffer = board.buffer.lock();
        assert_eq!(buffer.len(), 3);
    }
}

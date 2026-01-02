//! Playback file board implementation.
//!
//! Replays previously recorded BrainFlow data from CSV files.

use alloc::string::String;
use alloc::vec;
use alloc::vec::Vec;

use brainflow_sys::error::{Error, ErrorCode, Result};
use brainflow_sys::sync::Mutex;

use crate::board::{Board, BoardDescription, BoardState};
use crate::preset::Preset;
use crate::BoardId;

/// Playback file board for replaying recorded data.
pub struct PlaybackFileBoard {
    state: BoardState,
    file_path: String,
    master_board: BoardId,
    descriptions: [BoardDescription; 3],
    data: Mutex<Vec<Vec<f64>>>,
    current_row: Mutex<usize>,
    buffer: Mutex<Vec<f64>>,
    num_cols: usize,
}

impl PlaybackFileBoard {
    /// Create a new playback file board.
    pub fn new(file_path: String, master_board: BoardId) -> Self {
        // Get description from master board
        let desc = Self::get_master_description(master_board);
        let num_cols = desc.num_channels + 3; // channels + timestamp + marker + package

        Self {
            state: BoardState::NotReady,
            file_path,
            master_board,
            descriptions: [desc.clone(), desc.clone(), desc],
            data: Mutex::new(Vec::new()),
            current_row: Mutex::new(0),
            buffer: Mutex::new(Vec::new()),
            num_cols,
        }
    }

    /// Get board description from master board ID.
    fn get_master_description(board_id: BoardId) -> BoardDescription {
        match board_id {
            BoardId::Synthetic => BoardDescription {
                name: String::from("Synthetic Playback"),
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
                name: String::from("Cyton Playback"),
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
            _ => BoardDescription {
                name: String::from("Playback"),
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

    /// Parse CSV data from file contents.
    fn parse_csv(contents: &str) -> Vec<Vec<f64>> {
        let mut data = Vec::new();

        for line in contents.lines() {
            let line = line.trim();
            if line.is_empty() || line.starts_with('#') {
                continue;
            }

            let row: Vec<f64> = line
                .split(',')
                .filter_map(|s| s.trim().parse().ok())
                .collect();

            if !row.is_empty() {
                data.push(row);
            }
        }

        data
    }

    /// Get preset index.
    fn preset_index(preset: Preset) -> usize {
        match preset {
            Preset::Default => 0,
            Preset::Auxiliary => 1,
            Preset::Ancillary => 2,
        }
    }
}

impl Board for PlaybackFileBoard {
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

        // In a real implementation, we would read the file here
        // For no_std, file I/O would need to be provided externally
        #[cfg(feature = "std")]
        {
            use std::fs;
            let contents = fs::read_to_string(&self.file_path).map_err(|_| {
                Error::with_message(ErrorCode::GeneralError, "Failed to read file")
            })?;
            let data = Self::parse_csv(&contents);
            *self.data.lock() = data;
        }

        self.state = BoardState::Ready;
        Ok(())
    }

    fn release_session(&mut self) -> Result<()> {
        if self.state == BoardState::NotReady {
            return Err(Error::new(ErrorCode::BoardNotCreated));
        }

        self.state = BoardState::NotReady;
        *self.data.lock() = Vec::new();
        *self.current_row.lock() = 0;
        *self.buffer.lock() = Vec::new();
        Ok(())
    }

    fn start_stream(&mut self, _buffer_size: usize) -> Result<()> {
        if self.state != BoardState::Ready {
            return Err(Error::new(ErrorCode::BoardNotReady));
        }

        self.state = BoardState::Streaming;
        *self.current_row.lock() = 0;

        // Pre-fill buffer with data
        let data = self.data.lock();
        let mut buffer = self.buffer.lock();
        buffer.clear();

        for row in data.iter() {
            // Pad or truncate row to expected columns
            for i in 0..self.num_cols {
                buffer.push(row.get(i).copied().unwrap_or(0.0));
            }
        }

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

        // Remove retrieved data from buffer
        buffer.drain(..end);

        Ok(result)
    }

    fn get_board_data_count(&self, _preset: Preset) -> Result<usize> {
        let buffer = self.buffer.lock();
        Ok(buffer.len() / self.num_cols)
    }

    fn insert_marker(&mut self, _value: f64, _preset: Preset) -> Result<()> {
        // Markers don't make sense for playback
        Ok(())
    }

    fn config_board(&mut self, config: &str) -> Result<String> {
        // Handle configuration commands
        if config == "looping:true" {
            Ok(String::from("looping enabled"))
        } else if config == "looping:false" {
            Ok(String::from("looping disabled"))
        } else {
            Ok(String::new())
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_csv() {
        let csv = "1.0,2.0,3.0\n4.0,5.0,6.0\n";
        let data = PlaybackFileBoard::parse_csv(csv);
        assert_eq!(data.len(), 2);
        assert_eq!(data[0], vec![1.0, 2.0, 3.0]);
        assert_eq!(data[1], vec![4.0, 5.0, 6.0]);
    }

    #[test]
    fn test_parse_csv_with_comments() {
        let csv = "# header\n1.0,2.0\n\n3.0,4.0\n";
        let data = PlaybackFileBoard::parse_csv(csv);
        assert_eq!(data.len(), 2);
    }
}

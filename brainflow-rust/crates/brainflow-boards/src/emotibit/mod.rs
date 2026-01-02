//! EmotiBit board implementation.
//!
//! EmotiBit is a wearable biometric sensor that streams data over WiFi.
//! It provides PPG, EDA, temperature, accelerometer, gyroscope, and magnetometer data.

use alloc::boxed::Box;
use alloc::collections::VecDeque;
use alloc::string::String;
use alloc::vec;
use alloc::vec::Vec;
use core::sync::atomic::{AtomicBool, Ordering};

use brainflow_io::RingBuffer;
use brainflow_sys::error::{Error, ErrorCode, Result};

use crate::board::{reshape_to_row_major, Board, BoardDescription, BoardState};
use crate::preset::Preset;

/// EmotiBit default UDP port.
const EMOTIBIT_UDP_PORT: u16 = 12345;

/// EmotiBit board (wearable biometric sensor).
///
/// Streams biometric data over WiFi using a custom UDP protocol.
/// Provides PPG, EDA, temperature, and motion data.
pub struct EmotiBitBoard {
    state: BoardState,
    descriptions: [BoardDescription; 3],
    buffers: [Option<RingBuffer<f64>>; 3],
    marker_queue: VecDeque<f64>,
    running: AtomicBool,
    sample_count: u64,
    ip_address: String,
    port: u16,
}

impl EmotiBitBoard {
    /// EmotiBit default sampling rate for main sensors.
    pub const SAMPLING_RATE: u32 = 25;

    /// Auxiliary sampling rate (accelerometer/gyro).
    pub const AUX_SAMPLING_RATE: u32 = 25;

    /// PPG sampling rate.
    pub const PPG_SAMPLING_RATE: u32 = 25;

    /// Create a new EmotiBit board.
    pub fn new(ip_address: Option<&str>, port: Option<u16>) -> Self {
        Self {
            state: BoardState::NotReady,
            descriptions: [
                Self::create_default_description(),
                Self::create_auxiliary_description(),
                Self::create_ancillary_description(),
            ],
            buffers: [None, None, None],
            marker_queue: VecDeque::new(),
            running: AtomicBool::new(false),
            sample_count: 0,
            ip_address: String::from(ip_address.unwrap_or("0.0.0.0")),
            port: port.unwrap_or(EMOTIBIT_UDP_PORT),
        }
    }

    fn create_default_description() -> BoardDescription {
        // Default preset: PPG, EDA, Temperature
        // Channels: package, PPG_IR, PPG_RED, PPG_GREEN, EDA, Temperature, Humidity, timestamp, marker
        let num_channels = 9;

        BoardDescription {
            sampling_rate: Self::SAMPLING_RATE,
            num_channels,
            eeg_channels: vec![],
            emg_channels: vec![],
            ecg_channels: vec![],
            eog_channels: vec![],
            ppg_channels: vec![1, 2, 3], // IR, Red, Green
            eda_channels: vec![4],
            accel_channels: vec![],
            gyro_channels: vec![],
            temperature_channels: vec![5],
            battery_channel: None,
            timestamp_channel: Some(7),
            marker_channel: Some(8),
            package_num_channel: Some(0),
            name: String::from("EmotiBit"),
        }
    }

    fn create_auxiliary_description() -> BoardDescription {
        // Auxiliary preset: Motion data (accelerometer, gyroscope, magnetometer)
        // Channels: package, accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z, mag_x, mag_y, mag_z, timestamp
        let num_channels = 11;

        BoardDescription {
            sampling_rate: Self::AUX_SAMPLING_RATE,
            num_channels,
            eeg_channels: vec![],
            emg_channels: vec![],
            ecg_channels: vec![],
            eog_channels: vec![],
            ppg_channels: vec![],
            eda_channels: vec![],
            accel_channels: vec![1, 2, 3],
            gyro_channels: vec![4, 5, 6],
            temperature_channels: vec![],
            battery_channel: None,
            timestamp_channel: Some(10),
            marker_channel: None,
            package_num_channel: Some(0),
            name: String::from("EmotiBit Aux"),
        }
    }

    fn create_ancillary_description() -> BoardDescription {
        // Ancillary preset: Additional sensor data
        // Channels: package, battery, thermopile, timestamp
        let num_channels = 4;

        BoardDescription {
            sampling_rate: Self::SAMPLING_RATE,
            num_channels,
            eeg_channels: vec![],
            emg_channels: vec![],
            ecg_channels: vec![],
            eog_channels: vec![],
            ppg_channels: vec![],
            eda_channels: vec![],
            accel_channels: vec![],
            gyro_channels: vec![],
            temperature_channels: vec![2],
            battery_channel: Some(1),
            timestamp_channel: Some(3),
            marker_channel: None,
            package_num_channel: Some(0),
            name: String::from("EmotiBit Ancillary"),
        }
    }

    fn preset_index(preset: Preset) -> usize {
        match preset {
            Preset::Default => 0,
            Preset::Auxiliary => 1,
            Preset::Ancillary => 2,
        }
    }

    /// Parse EmotiBit data packet.
    ///
    /// EmotiBit uses a text-based protocol with comma-separated values.
    /// Format: timestamp,packetNumber,dataType,dataVersion,dataCount,data...
    #[allow(dead_code)]
    fn parse_packet(&self, data: &str) -> Option<(String, Vec<f64>)> {
        let parts: Vec<&str> = data.split(',').collect();
        if parts.len() < 6 {
            return None;
        }

        let data_type = parts[2];
        let data_count: usize = parts[4].parse().ok()?;

        if parts.len() < 5 + data_count {
            return None;
        }

        let values: Vec<f64> = parts[5..5 + data_count]
            .iter()
            .filter_map(|s| s.parse().ok())
            .collect();

        Some((String::from(data_type), values))
    }
}

impl Board for EmotiBitBoard {
    fn state(&self) -> BoardState {
        self.state
    }

    fn description(&self, preset: Preset) -> &BoardDescription {
        &self.descriptions[Self::preset_index(preset)]
    }

    fn prepare_session(&mut self) -> Result<()> {
        if self.state != BoardState::NotReady {
            return Err(Error::new(ErrorCode::PortAlreadyOpen));
        }

        // In a real implementation, we would:
        // 1. Create UDP socket to receive EmotiBit data
        // 2. Send discovery/connection message

        self.state = BoardState::Ready;
        Ok(())
    }

    fn release_session(&mut self) -> Result<()> {
        if self.state == BoardState::Streaming {
            self.stop_stream()?;
        }

        self.state = BoardState::NotReady;
        self.buffers = [None, None, None];
        self.marker_queue.clear();
        self.sample_count = 0;

        Ok(())
    }

    fn start_stream(&mut self, buffer_size: usize) -> Result<()> {
        if self.state != BoardState::Ready {
            return Err(Error::new(ErrorCode::BoardNotReady));
        }

        // Create ring buffers for each preset
        for (i, desc) in self.descriptions.iter().enumerate() {
            if desc.num_channels > 0 {
                self.buffers[i] = Some(RingBuffer::new(buffer_size, desc.num_channels));
            }
        }

        self.running.store(true, Ordering::Release);
        self.state = BoardState::Streaming;

        Ok(())
    }

    fn stop_stream(&mut self) -> Result<()> {
        if self.state != BoardState::Streaming {
            return Err(Error::new(ErrorCode::StreamThreadNotRunning));
        }

        self.running.store(false, Ordering::Release);
        self.state = BoardState::Ready;

        Ok(())
    }

    fn get_current_board_data(&self, num_samples: usize, preset: Preset) -> Result<Vec<f64>> {
        let idx = Self::preset_index(preset);

        if let Some(ref buffer) = self.buffers[idx] {
            let data = buffer.peek(num_samples);
            let num_channels = self.descriptions[idx].num_channels;
            Ok(reshape_to_row_major(&data, num_channels))
        } else {
            Err(Error::new(ErrorCode::BoardNotReady))
        }
    }

    fn get_board_data(&self, num_samples: Option<usize>, preset: Preset) -> Result<Vec<f64>> {
        let idx = Self::preset_index(preset);

        if let Some(ref buffer) = self.buffers[idx] {
            let count = buffer.len();
            let to_get = num_samples.unwrap_or(count).min(count);
            let data = buffer.pop(to_get);
            let num_channels = self.descriptions[idx].num_channels;
            Ok(reshape_to_row_major(&data, num_channels))
        } else {
            Err(Error::new(ErrorCode::BoardNotReady))
        }
    }

    fn get_board_data_count(&self, preset: Preset) -> Result<usize> {
        let idx = Self::preset_index(preset);

        if let Some(ref buffer) = self.buffers[idx] {
            Ok(buffer.len())
        } else {
            Err(Error::new(ErrorCode::BoardNotReady))
        }
    }

    fn insert_marker(&mut self, value: f64, _preset: Preset) -> Result<()> {
        self.marker_queue.push_back(value);
        Ok(())
    }

    fn config_board(&mut self, _config: &str) -> Result<String> {
        // EmotiBit configuration would be sent via its protocol
        Ok(String::new())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_emotibit_creation() {
        let board = EmotiBitBoard::new(None, None);
        assert_eq!(board.state(), BoardState::NotReady);

        let desc = board.description(Preset::Default);
        assert_eq!(desc.sampling_rate, 25);
        assert_eq!(desc.ppg_channels.len(), 3);
        assert_eq!(desc.eda_channels.len(), 1);
    }

    #[test]
    fn test_emotibit_auxiliary() {
        let board = EmotiBitBoard::new(None, None);

        let desc = board.description(Preset::Auxiliary);
        assert_eq!(desc.accel_channels.len(), 3);
        assert_eq!(desc.gyro_channels.len(), 3);
    }

    #[test]
    fn test_emotibit_lifecycle() {
        let mut board = EmotiBitBoard::new(Some("192.168.1.100"), Some(12345));

        board.prepare_session().unwrap();
        assert_eq!(board.state(), BoardState::Ready);

        board.start_stream(1000).unwrap();
        assert_eq!(board.state(), BoardState::Streaming);

        board.stop_stream().unwrap();
        assert_eq!(board.state(), BoardState::Ready);

        board.release_session().unwrap();
        assert_eq!(board.state(), BoardState::NotReady);
    }
}

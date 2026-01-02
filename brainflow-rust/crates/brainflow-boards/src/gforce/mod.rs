//! gForce board implementations.
//!
//! gForce devices are gesture recognition armband sensors that stream EMG data.

use alloc::collections::VecDeque;
use alloc::string::String;
use alloc::vec;
use alloc::vec::Vec;
use core::sync::atomic::{AtomicBool, AtomicU64, Ordering};

use brainflow_io::RingBuffer;
use brainflow_sys::error::{Error, ErrorCode, Result};

use crate::board::{reshape_to_row_major, Board, BoardDescription, BoardState};
use crate::preset::Preset;

/// gForce variant.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum GforceVariant {
    /// gForce Pro (8 EMG channels).
    Pro,
    /// gForce Dual (2 devices, 16 EMG channels total).
    Dual,
}

impl GforceVariant {
    /// Get the number of EMG channels.
    pub fn num_channels(&self) -> usize {
        match self {
            GforceVariant::Pro => 8,
            GforceVariant::Dual => 16,
        }
    }
}

/// gForce EMG armband.
///
/// Gesture recognition device with EMG sensors.
pub struct GforceBoard {
    variant: GforceVariant,
    state: BoardState,
    descriptions: [BoardDescription; 3],
    buffers: [Option<RingBuffer<f64>>; 3],
    marker_queue: VecDeque<f64>,
    running: AtomicBool,
    sample_count: AtomicU64,
}

impl GforceBoard {
    /// gForce sampling rate.
    pub const SAMPLING_RATE: u32 = 500;

    /// Create a new gForce board.
    pub fn new(variant: GforceVariant) -> Self {
        let descriptions = [
            Self::create_default_description(variant),
            Self::create_auxiliary_description(),
            Self::create_ancillary_description(),
        ];

        Self {
            variant,
            state: BoardState::NotReady,
            descriptions,
            buffers: [None, None, None],
            marker_queue: VecDeque::new(),
            running: AtomicBool::new(false),
            sample_count: AtomicU64::new(0),
        }
    }

    fn create_default_description(variant: GforceVariant) -> BoardDescription {
        let num_emg = variant.num_channels();
        // Channels: package, EMG..., timestamp, marker
        let num_channels = 1 + num_emg + 2;

        BoardDescription {
            sampling_rate: Self::SAMPLING_RATE,
            num_channels,
            eeg_channels: vec![],
            emg_channels: (1..=num_emg).collect(),
            ecg_channels: vec![],
            eog_channels: vec![],
            ppg_channels: vec![],
            eda_channels: vec![],
            accel_channels: vec![],
            gyro_channels: vec![],
            temperature_channels: vec![],
            battery_channel: None,
            timestamp_channel: Some(num_emg + 1),
            marker_channel: Some(num_emg + 2),
            package_num_channel: Some(0),
            name: String::from(match variant {
                GforceVariant::Pro => "gForce Pro",
                GforceVariant::Dual => "gForce Dual",
            }),
        }
    }

    fn create_auxiliary_description() -> BoardDescription {
        // Auxiliary preset: Quaternion and gesture data
        BoardDescription {
            sampling_rate: 50,
            num_channels: 6,
            eeg_channels: vec![],
            emg_channels: vec![],
            ecg_channels: vec![],
            eog_channels: vec![],
            ppg_channels: vec![],
            eda_channels: vec![],
            accel_channels: vec![1, 2, 3],
            gyro_channels: vec![],
            temperature_channels: vec![],
            battery_channel: None,
            timestamp_channel: Some(5),
            marker_channel: None,
            package_num_channel: Some(0),
            name: String::from("gForce Aux"),
        }
    }

    fn create_ancillary_description() -> BoardDescription {
        BoardDescription {
            sampling_rate: 1,
            num_channels: 3,
            eeg_channels: vec![],
            emg_channels: vec![],
            ecg_channels: vec![],
            eog_channels: vec![],
            ppg_channels: vec![],
            eda_channels: vec![],
            accel_channels: vec![],
            gyro_channels: vec![],
            temperature_channels: vec![],
            battery_channel: Some(1),
            timestamp_channel: Some(2),
            marker_channel: None,
            package_num_channel: Some(0),
            name: String::from("gForce Ancillary"),
        }
    }

    fn preset_index(preset: Preset) -> usize {
        match preset {
            Preset::Default => 0,
            Preset::Auxiliary => 1,
            Preset::Ancillary => 2,
        }
    }

    /// Get the variant type.
    pub fn variant(&self) -> GforceVariant {
        self.variant
    }
}

impl Board for GforceBoard {
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
        self.sample_count.store(0, Ordering::Relaxed);
        Ok(())
    }

    fn start_stream(&mut self, buffer_size: usize) -> Result<()> {
        if self.state != BoardState::Ready {
            return Err(Error::new(ErrorCode::BoardNotReady));
        }

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
        Ok(String::new())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_gforce_variants() {
        assert_eq!(GforceVariant::Pro.num_channels(), 8);
        assert_eq!(GforceVariant::Dual.num_channels(), 16);
    }

    #[test]
    fn test_gforce_creation() {
        let board = GforceBoard::new(GforceVariant::Pro);
        assert_eq!(board.state(), BoardState::NotReady);

        let desc = board.description(Preset::Default);
        assert_eq!(desc.sampling_rate, 500);
        assert_eq!(desc.emg_channels.len(), 8);
    }
}

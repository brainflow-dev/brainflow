//! BoardShim - High-level board interface.

use alloc::boxed::Box;
use alloc::string::String;
use alloc::vec::Vec;

use brainflow_boards::{
    Board, BoardController, BoardDescription, BoardId, BoardParams, Preset,
    board::BoardState,
    synthetic::SyntheticBoard,
};
use brainflow_sys::error::{Error, ErrorCode, Result};

/// High-level interface for board communication.
///
/// This matches the C++ BoardShim API.
pub struct BoardShim {
    board_id: BoardId,
    params: BoardParams,
    board: Option<Box<dyn Board>>,
}

impl BoardShim {
    /// Create a new BoardShim.
    pub fn new(board_id: BoardId, params: BoardParams) -> Result<Self> {
        Ok(Self {
            board_id,
            params,
            board: None,
        })
    }

    /// Get the board ID.
    pub fn board_id(&self) -> BoardId {
        self.board_id
    }

    /// Prepare the session (initialize hardware).
    pub fn prepare_session(&mut self) -> Result<()> {
        if self.board.is_some() {
            return Err(Error::new(ErrorCode::PortAlreadyOpen));
        }

        let mut board: Box<dyn Board> = match self.board_id {
            BoardId::Synthetic => Box::new(SyntheticBoard::new()),
            _ => {
                return Err(Error::with_message(
                    ErrorCode::UnsupportedBoard,
                    "Board not yet implemented",
                ))
            }
        };

        board.prepare_session()?;
        self.board = Some(board);
        Ok(())
    }

    /// Release the session (cleanup and disconnect).
    pub fn release_session(&mut self) -> Result<()> {
        if let Some(mut board) = self.board.take() {
            board.release_session()
        } else {
            Err(Error::new(ErrorCode::BoardNotCreated))
        }
    }

    /// Start data streaming.
    ///
    /// # Arguments
    /// * `buffer_size` - Size of the ring buffer in samples.
    pub fn start_stream(&mut self, buffer_size: usize) -> Result<()> {
        if let Some(ref mut board) = self.board {
            board.start_stream(buffer_size)
        } else {
            Err(Error::new(ErrorCode::BoardNotCreated))
        }
    }

    /// Stop data streaming.
    pub fn stop_stream(&mut self) -> Result<()> {
        if let Some(ref mut board) = self.board {
            board.stop_stream()
        } else {
            Err(Error::new(ErrorCode::BoardNotCreated))
        }
    }

    /// Get current board data without removing it from the buffer.
    pub fn get_current_board_data(&self, num_samples: usize) -> Result<Vec<f64>> {
        self.get_current_board_data_preset(num_samples, Preset::Default)
    }

    /// Get current board data for a specific preset.
    pub fn get_current_board_data_preset(
        &self,
        num_samples: usize,
        preset: Preset,
    ) -> Result<Vec<f64>> {
        if let Some(ref board) = self.board {
            board.get_current_board_data(num_samples, preset)
        } else {
            Err(Error::new(ErrorCode::BoardNotCreated))
        }
    }

    /// Get board data and remove it from the buffer.
    pub fn get_board_data(&self, num_samples: Option<usize>) -> Result<Vec<f64>> {
        self.get_board_data_preset(num_samples, Preset::Default)
    }

    /// Get board data for a specific preset.
    pub fn get_board_data_preset(
        &self,
        num_samples: Option<usize>,
        preset: Preset,
    ) -> Result<Vec<f64>> {
        if let Some(ref board) = self.board {
            board.get_board_data(num_samples, preset)
        } else {
            Err(Error::new(ErrorCode::BoardNotCreated))
        }
    }

    /// Get the number of samples in the buffer.
    pub fn get_board_data_count(&self) -> Result<usize> {
        self.get_board_data_count_preset(Preset::Default)
    }

    /// Get the number of samples for a specific preset.
    pub fn get_board_data_count_preset(&self, preset: Preset) -> Result<usize> {
        if let Some(ref board) = self.board {
            board.get_board_data_count(preset)
        } else {
            Err(Error::new(ErrorCode::BoardNotCreated))
        }
    }

    /// Insert a marker into the data stream.
    pub fn insert_marker(&mut self, value: f64) -> Result<()> {
        self.insert_marker_preset(value, Preset::Default)
    }

    /// Insert a marker for a specific preset.
    pub fn insert_marker_preset(&mut self, value: f64, preset: Preset) -> Result<()> {
        if let Some(ref mut board) = self.board {
            board.insert_marker(value, preset)
        } else {
            Err(Error::new(ErrorCode::BoardNotCreated))
        }
    }

    /// Send configuration to the board.
    pub fn config_board(&mut self, config: &str) -> Result<String> {
        if let Some(ref mut board) = self.board {
            board.config_board(config)
        } else {
            Err(Error::new(ErrorCode::BoardNotCreated))
        }
    }

    /// Check if the session is prepared.
    pub fn is_prepared(&self) -> bool {
        self.board.as_ref().map_or(false, |b| b.is_prepared())
    }

    /// Check if streaming is active.
    pub fn is_streaming(&self) -> bool {
        self.board.as_ref().map_or(false, |b| b.is_streaming())
    }

    /// Get the board description.
    pub fn get_board_description(&self) -> Result<BoardDescription> {
        self.get_board_description_preset(Preset::Default)
    }

    /// Get the board description for a specific preset.
    pub fn get_board_description_preset(&self, preset: Preset) -> Result<BoardDescription> {
        if let Some(ref board) = self.board {
            Ok(board.description(preset).clone())
        } else {
            Err(Error::new(ErrorCode::BoardNotCreated))
        }
    }

    /// Get the sampling rate.
    pub fn get_sampling_rate(&self) -> Result<u32> {
        Ok(self.get_board_description()?.sampling_rate)
    }

    /// Get EEG channel indices.
    pub fn get_eeg_channels(&self) -> Result<Vec<usize>> {
        Ok(self.get_board_description()?.eeg_channels)
    }

    /// Get accelerometer channel indices.
    pub fn get_accel_channels(&self) -> Result<Vec<usize>> {
        Ok(self.get_board_description()?.accel_channels)
    }

    /// Get gyroscope channel indices.
    pub fn get_gyro_channels(&self) -> Result<Vec<usize>> {
        Ok(self.get_board_description()?.gyro_channels)
    }

    /// Get the timestamp channel index.
    pub fn get_timestamp_channel(&self) -> Result<Option<usize>> {
        Ok(self.get_board_description()?.timestamp_channel)
    }

    /// Get the marker channel index.
    pub fn get_marker_channel(&self) -> Result<Option<usize>> {
        Ok(self.get_board_description()?.marker_channel)
    }

    /// Get the package number channel index.
    pub fn get_package_num_channel(&self) -> Result<Option<usize>> {
        Ok(self.get_board_description()?.package_num_channel)
    }

    /// Get the device name.
    pub fn get_device_name(&self) -> Result<String> {
        Ok(self.get_board_description()?.name)
    }
}

impl Drop for BoardShim {
    fn drop(&mut self) {
        // Best-effort cleanup
        let _ = self.release_session();
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_board_shim_lifecycle() {
        let mut board = BoardShim::new(BoardId::Synthetic, BoardParams::new()).unwrap();

        assert!(!board.is_prepared());

        board.prepare_session().unwrap();
        assert!(board.is_prepared());

        board.start_stream(1000).unwrap();
        assert!(board.is_streaming());

        let count = board.get_board_data_count().unwrap();
        assert!(count > 0);

        board.stop_stream().unwrap();
        assert!(!board.is_streaming());

        board.release_session().unwrap();
        assert!(!board.is_prepared());
    }
}

//! Board controller for managing board instances.

use alloc::boxed::Box;
use alloc::collections::BTreeMap;
use alloc::string::String;
use alloc::vec::Vec;

use brainflow_sys::error::{Error, ErrorCode, Result};
use brainflow_sys::sync::Mutex;

use crate::board::{Board, BoardDescription};
use crate::preset::Preset;
use crate::{BoardId, BoardParams};
use crate::synthetic::SyntheticBoard;

/// Global board controller.
///
/// Manages board instances using the factory pattern.
pub struct BoardController {
    boards: Mutex<BTreeMap<(i32, String), Box<dyn Board>>>,
}

impl BoardController {
    /// Create a new board controller.
    pub const fn new() -> Self {
        Self {
            boards: Mutex::new(BTreeMap::new()),
        }
    }

    /// Generate a key for the board map.
    fn make_key(board_id: BoardId, params: &BoardParams) -> (i32, String) {
        let params_str = alloc::format!(
            "{}:{}:{}:{}",
            params.serial_port.as_deref().unwrap_or(""),
            params.mac_address.as_deref().unwrap_or(""),
            params.ip_address.as_deref().unwrap_or(""),
            params.ip_port.unwrap_or(0)
        );
        (board_id.to_raw(), params_str)
    }

    /// Create a board instance based on board ID.
    fn create_board(board_id: BoardId, _params: &BoardParams) -> Result<Box<dyn Board>> {
        match board_id {
            BoardId::Synthetic => Ok(Box::new(SyntheticBoard::new())),
            // Add other board types here
            _ => Err(Error::with_message(
                ErrorCode::UnsupportedBoard,
                alloc::format!("Board {:?} not yet implemented", board_id),
            )),
        }
    }

    /// Prepare a session for the specified board.
    pub fn prepare_session(&self, board_id: BoardId, params: &BoardParams) -> Result<()> {
        let key = Self::make_key(board_id, params);

        let mut boards = self.boards.lock();

        if boards.contains_key(&key) {
            return Err(Error::new(ErrorCode::AnotherBoardCreated));
        }

        let mut board = Self::create_board(board_id, params)?;
        board.prepare_session()?;

        boards.insert(key, board);
        Ok(())
    }

    /// Release a session for the specified board.
    pub fn release_session(&self, board_id: BoardId, params: &BoardParams) -> Result<()> {
        let key = Self::make_key(board_id, params);

        let mut boards = self.boards.lock();

        if let Some(mut board) = boards.remove(&key) {
            board.release_session()
        } else {
            Err(Error::new(ErrorCode::BoardNotCreated))
        }
    }

    /// Start streaming for the specified board.
    pub fn start_stream(
        &self,
        board_id: BoardId,
        params: &BoardParams,
        buffer_size: usize,
    ) -> Result<()> {
        let key = Self::make_key(board_id, params);

        let mut boards = self.boards.lock();

        if let Some(board) = boards.get_mut(&key) {
            board.start_stream(buffer_size)
        } else {
            Err(Error::new(ErrorCode::BoardNotCreated))
        }
    }

    /// Stop streaming for the specified board.
    pub fn stop_stream(&self, board_id: BoardId, params: &BoardParams) -> Result<()> {
        let key = Self::make_key(board_id, params);

        let mut boards = self.boards.lock();

        if let Some(board) = boards.get_mut(&key) {
            board.stop_stream()
        } else {
            Err(Error::new(ErrorCode::BoardNotCreated))
        }
    }

    /// Get current board data.
    pub fn get_current_board_data(
        &self,
        board_id: BoardId,
        params: &BoardParams,
        num_samples: usize,
        preset: Preset,
    ) -> Result<Vec<f64>> {
        let key = Self::make_key(board_id, params);

        let boards = self.boards.lock();

        if let Some(board) = boards.get(&key) {
            board.get_current_board_data(num_samples, preset)
        } else {
            Err(Error::new(ErrorCode::BoardNotCreated))
        }
    }

    /// Get board data (removes from buffer).
    pub fn get_board_data(
        &self,
        board_id: BoardId,
        params: &BoardParams,
        num_samples: Option<usize>,
        preset: Preset,
    ) -> Result<Vec<f64>> {
        let key = Self::make_key(board_id, params);

        let boards = self.boards.lock();

        if let Some(board) = boards.get(&key) {
            board.get_board_data(num_samples, preset)
        } else {
            Err(Error::new(ErrorCode::BoardNotCreated))
        }
    }

    /// Get the number of samples in the buffer.
    pub fn get_board_data_count(
        &self,
        board_id: BoardId,
        params: &BoardParams,
        preset: Preset,
    ) -> Result<usize> {
        let key = Self::make_key(board_id, params);

        let boards = self.boards.lock();

        if let Some(board) = boards.get(&key) {
            board.get_board_data_count(preset)
        } else {
            Err(Error::new(ErrorCode::BoardNotCreated))
        }
    }

    /// Insert a marker into the data stream.
    pub fn insert_marker(
        &self,
        board_id: BoardId,
        params: &BoardParams,
        value: f64,
        preset: Preset,
    ) -> Result<()> {
        let key = Self::make_key(board_id, params);

        let mut boards = self.boards.lock();

        if let Some(board) = boards.get_mut(&key) {
            board.insert_marker(value, preset)
        } else {
            Err(Error::new(ErrorCode::BoardNotCreated))
        }
    }

    /// Send configuration to the board.
    pub fn config_board(
        &self,
        board_id: BoardId,
        params: &BoardParams,
        config: &str,
    ) -> Result<String> {
        let key = Self::make_key(board_id, params);

        let mut boards = self.boards.lock();

        if let Some(board) = boards.get_mut(&key) {
            board.config_board(config)
        } else {
            Err(Error::new(ErrorCode::BoardNotCreated))
        }
    }

    /// Get board description.
    pub fn get_board_description(
        &self,
        board_id: BoardId,
        params: &BoardParams,
        preset: Preset,
    ) -> Result<BoardDescription> {
        let key = Self::make_key(board_id, params);

        let boards = self.boards.lock();

        if let Some(board) = boards.get(&key) {
            Ok(board.description(preset).clone())
        } else {
            Err(Error::new(ErrorCode::BoardNotCreated))
        }
    }

    /// Check if a board is prepared.
    pub fn is_prepared(&self, board_id: BoardId, params: &BoardParams) -> bool {
        let key = Self::make_key(board_id, params);
        let boards = self.boards.lock();
        boards.contains_key(&key)
    }
}

impl Default for BoardController {
    fn default() -> Self {
        Self::new()
    }
}

/// Global board controller instance.
#[cfg(feature = "std")]
static CONTROLLER: BoardController = BoardController::new();

/// Get the global board controller.
#[cfg(feature = "std")]
pub fn controller() -> &'static BoardController {
    &CONTROLLER
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_controller_synthetic() {
        let controller = BoardController::new();
        let params = BoardParams::new();

        // Prepare
        controller.prepare_session(BoardId::Synthetic, &params).unwrap();
        assert!(controller.is_prepared(BoardId::Synthetic, &params));

        // Start stream
        controller.start_stream(BoardId::Synthetic, &params, 1000).unwrap();

        // Get data count
        let count = controller.get_board_data_count(BoardId::Synthetic, &params, Preset::Default).unwrap();
        assert!(count > 0);

        // Stop stream
        controller.stop_stream(BoardId::Synthetic, &params).unwrap();

        // Release
        controller.release_session(BoardId::Synthetic, &params).unwrap();
        assert!(!controller.is_prepared(BoardId::Synthetic, &params));
    }
}

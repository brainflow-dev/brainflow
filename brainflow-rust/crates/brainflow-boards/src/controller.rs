//! Board controller for managing board instances.

use alloc::boxed::Box;
use alloc::collections::BTreeMap;
use alloc::string::String;
use alloc::vec::Vec;

use brainflow_sys::error::{Error, ErrorCode, Result};
use brainflow_sys::sync::Mutex;

use crate::ant_neuro::{AntNeuroBoard, AntNeuroVariant};
use crate::board::{Board, BoardDescription};
use crate::brainbit::{BrainBitBoard, BrainBitVariant};
use crate::emotibit::EmotiBitBoard;
use crate::explore::{ExploreBoard, ExploreVariant};
use crate::galea::GaleaBoard;
use crate::ganglion::GanglionBoard;
use crate::gforce::{GforceBoard, GforceVariant};
use crate::misc::{
    AavaaV3Board, BrainAliveBoard, CallibriBoard, CallibriVariant, EnophoneBoard, FasciaBoard,
    GanglionNativeBoard, IronbciBoard, NeuropawnKnightBoard, NtlWifiBoard, PieegBoard,
    SynchroniBoard, UnicornBoard,
};
use crate::muse::{MuseBoard, MuseVariant};
use crate::neurosity::{NeurosityBoard, NeurosityVariant};
use crate::openbci::{
    CytonBoard, CytonDaisyBoard, CytonDaisyWifiBoard, CytonWifiBoard, FreeEeg128Board,
    FreeEeg32Board, GanglionWifiBoard,
};
use crate::playback::PlaybackFileBoard;
use crate::preset::Preset;
use crate::streaming::StreamingBoard;
use crate::synthetic::SyntheticBoard;
use crate::{BoardId, BoardParams};

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
    fn create_board(board_id: BoardId, params: &BoardParams) -> Result<Box<dyn Board>> {
        match board_id {
            // Utility boards
            BoardId::PlaybackFile => {
                let file = params.file.as_ref().ok_or_else(|| {
                    Error::with_message(ErrorCode::InvalidArguments, "file path required")
                })?;
                let master_board = params.master_board.ok_or_else(|| {
                    Error::with_message(ErrorCode::InvalidArguments, "master_board required")
                })?;
                Ok(Box::new(PlaybackFileBoard::new(file.clone(), master_board)))
            }
            BoardId::Streaming => {
                let ip = params.ip_address.as_ref().ok_or_else(|| {
                    Error::with_message(ErrorCode::InvalidArguments, "ip_address required")
                })?;
                let port = params.ip_port.ok_or_else(|| {
                    Error::with_message(ErrorCode::InvalidArguments, "ip_port required")
                })?;
                let master_board = params.master_board.ok_or_else(|| {
                    Error::with_message(ErrorCode::InvalidArguments, "master_board required")
                })?;
                Ok(Box::new(StreamingBoard::new(ip.clone(), port, master_board)))
            }
            BoardId::Synthetic => Ok(Box::new(SyntheticBoard::new())),

            // OpenBCI serial boards
            BoardId::Cyton => {
                let port = params.serial_port.as_ref().ok_or_else(|| {
                    Error::with_message(ErrorCode::InvalidArguments, "serial_port required")
                })?;
                Ok(Box::new(CytonBoard::new(port)))
            }
            BoardId::CytonDaisy => {
                let port = params.serial_port.as_ref().ok_or_else(|| {
                    Error::with_message(ErrorCode::InvalidArguments, "serial_port required")
                })?;
                Ok(Box::new(CytonDaisyBoard::new(port)))
            }
            BoardId::FreeEeg32 => {
                let port = params.serial_port.as_ref().ok_or_else(|| {
                    Error::with_message(ErrorCode::InvalidArguments, "serial_port required")
                })?;
                Ok(Box::new(FreeEeg32Board::new(port)))
            }
            BoardId::FreeEeg128 => {
                let port = params.serial_port.as_ref().ok_or_else(|| {
                    Error::with_message(ErrorCode::InvalidArguments, "serial_port required")
                })?;
                Ok(Box::new(FreeEeg128Board::new(port)))
            }

            // OpenBCI WiFi boards
            BoardId::CytonWifi => {
                let ip = params.ip_address.as_ref().ok_or_else(|| {
                    Error::with_message(ErrorCode::InvalidArguments, "ip_address required")
                })?;
                Ok(Box::new(CytonWifiBoard::new(ip, params.ip_port)))
            }
            BoardId::CytonDaisyWifi => {
                let ip = params.ip_address.as_ref().ok_or_else(|| {
                    Error::with_message(ErrorCode::InvalidArguments, "ip_address required")
                })?;
                Ok(Box::new(CytonDaisyWifiBoard::new(ip, params.ip_port)))
            }
            BoardId::GanglionWifi => {
                let ip = params.ip_address.as_ref().ok_or_else(|| {
                    Error::with_message(ErrorCode::InvalidArguments, "ip_address required")
                })?;
                Ok(Box::new(GanglionWifiBoard::new(ip, params.ip_port)))
            }

            // BLE boards
            BoardId::Ganglion => Ok(Box::new(GanglionBoard::new())),
            BoardId::Muse2 => Ok(Box::new(MuseBoard::new(MuseVariant::Muse2))),
            BoardId::MuseS => Ok(Box::new(MuseBoard::new(MuseVariant::MuseS))),
            BoardId::Muse2016 => Ok(Box::new(MuseBoard::new(MuseVariant::Muse2016))),
            BoardId::Muse2Bled => Ok(Box::new(MuseBoard::new(MuseVariant::Muse2))),
            BoardId::MuseSBled => Ok(Box::new(MuseBoard::new(MuseVariant::MuseS))),
            BoardId::Muse2016Bled => Ok(Box::new(MuseBoard::new(MuseVariant::Muse2016))),

            // Galea (UDP variants)
            BoardId::Galea | BoardId::GaleaV4 => {
                let ip = params.ip_address.as_ref().ok_or_else(|| {
                    Error::with_message(ErrorCode::InvalidArguments, "ip_address required")
                })?;
                Ok(Box::new(GaleaBoard::new(ip, params.ip_port)))
            }
            BoardId::GaleaSerial | BoardId::GaleaV4Serial => {
                let port = params.serial_port.as_ref().ok_or_else(|| {
                    Error::with_message(ErrorCode::InvalidArguments, "serial_port required")
                })?;
                // Serial variants use GaleaBoard with a local address (for now)
                Ok(Box::new(GaleaBoard::new("127.0.0.1", None)))
            }

            // ANT Neuro boards
            BoardId::AntNeuroEe410 => Ok(Box::new(AntNeuroBoard::new(AntNeuroVariant::Ee410))),
            BoardId::AntNeuroEe411 => Ok(Box::new(AntNeuroBoard::new(AntNeuroVariant::Ee411))),
            BoardId::AntNeuroEe430 => Ok(Box::new(AntNeuroBoard::new(AntNeuroVariant::Ee430))),
            BoardId::AntNeuroEe211 => Ok(Box::new(AntNeuroBoard::new(AntNeuroVariant::Ee211))),
            BoardId::AntNeuroEe212 => Ok(Box::new(AntNeuroBoard::new(AntNeuroVariant::Ee212))),
            BoardId::AntNeuroEe213 => Ok(Box::new(AntNeuroBoard::new(AntNeuroVariant::Ee213))),
            BoardId::AntNeuroEe214 => Ok(Box::new(AntNeuroBoard::new(AntNeuroVariant::Ee214))),
            BoardId::AntNeuroEe215 => Ok(Box::new(AntNeuroBoard::new(AntNeuroVariant::Ee215))),
            BoardId::AntNeuroEe221 => Ok(Box::new(AntNeuroBoard::new(AntNeuroVariant::Ee221))),
            BoardId::AntNeuroEe222 => Ok(Box::new(AntNeuroBoard::new(AntNeuroVariant::Ee222))),
            BoardId::AntNeuroEe223 => Ok(Box::new(AntNeuroBoard::new(AntNeuroVariant::Ee223))),
            BoardId::AntNeuroEe224 => Ok(Box::new(AntNeuroBoard::new(AntNeuroVariant::Ee224))),
            BoardId::AntNeuroEe225 => Ok(Box::new(AntNeuroBoard::new(AntNeuroVariant::Ee225))),
            BoardId::AntNeuroEe511 => Ok(Box::new(AntNeuroBoard::new(AntNeuroVariant::Ee511))),

            // Neurosity boards (OSC protocol)
            BoardId::Notion1 => {
                let ip = params.ip_address.as_deref();
                let port = params.ip_port;
                Ok(Box::new(NeurosityBoard::new(NeurosityVariant::Notion1, ip, port)))
            }
            BoardId::Notion2 => {
                let ip = params.ip_address.as_deref();
                let port = params.ip_port;
                Ok(Box::new(NeurosityBoard::new(NeurosityVariant::Notion2, ip, port)))
            }
            BoardId::Crown => {
                let ip = params.ip_address.as_deref();
                let port = params.ip_port;
                Ok(Box::new(NeurosityBoard::new(NeurosityVariant::Crown, ip, port)))
            }

            // EmotiBit (WiFi biometric sensor)
            BoardId::Emotibit => {
                let ip = params.ip_address.as_deref();
                let port = params.ip_port;
                Ok(Box::new(EmotiBitBoard::new(ip, port)))
            }

            // BrainBit boards (BLE)
            BoardId::BrainBit => Ok(Box::new(BrainBitBoard::new(BrainBitVariant::Standard))),
            BoardId::BrainBitBled => Ok(Box::new(BrainBitBoard::new(BrainBitVariant::Bled))),

            // Explore boards (BLE)
            BoardId::Explore4Chan => Ok(Box::new(ExploreBoard::new(ExploreVariant::Chan4))),
            BoardId::Explore8Chan => Ok(Box::new(ExploreBoard::new(ExploreVariant::Chan8))),
            BoardId::Explore32Chan => Ok(Box::new(ExploreBoard::new(ExploreVariant::Chan32))),
            BoardId::ExplorePlus8Chan => Ok(Box::new(ExploreBoard::new(ExploreVariant::Plus8))),
            BoardId::ExplorePlus32Chan => Ok(Box::new(ExploreBoard::new(ExploreVariant::Plus32))),

            // gForce boards (BLE/WiFi)
            BoardId::GforcePro => Ok(Box::new(GforceBoard::new(GforceVariant::Pro))),
            BoardId::GforceDual => Ok(Box::new(GforceBoard::new(GforceVariant::Dual))),

            // Unicorn (requires g.tec SDK for full functionality)
            BoardId::Unicorn => Ok(Box::new(UnicornBoard::new())),

            // Callibri (requires NeuroMD SDK for full functionality)
            BoardId::CallibriEeg => Ok(Box::new(CallibriBoard::new(CallibriVariant::Eeg))),
            BoardId::CallibriEmg => Ok(Box::new(CallibriBoard::new(CallibriVariant::Emg))),
            BoardId::CallibriEcg => Ok(Box::new(CallibriBoard::new(CallibriVariant::Ecg))),

            // Other boards
            BoardId::Pieeg => Ok(Box::new(PieegBoard::new())),
            BoardId::Ironbci => Ok(Box::new(IronbciBoard::new())),
            BoardId::Fascia => Ok(Box::new(FasciaBoard::new())),
            BoardId::Enophone => Ok(Box::new(EnophoneBoard::new())),
            BoardId::BrainAlive => Ok(Box::new(BrainAliveBoard::new())),
            BoardId::GanglionNative => Ok(Box::new(GanglionNativeBoard::new())),
            BoardId::NtlWifi => Ok(Box::new(NtlWifiBoard::new())),
            BoardId::AavaaV3 => Ok(Box::new(AavaaV3Board::new())),
            BoardId::NeuropawnKnight => Ok(Box::new(NeuropawnKnightBoard::new())),
            BoardId::Synchroni => Ok(Box::new(SynchroniBoard::new())),

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

    #[test]
    fn test_board_factory_requires_params() {
        let controller = BoardController::new();

        // Cyton requires serial_port
        let params = BoardParams::new();
        let result = controller.prepare_session(BoardId::Cyton, &params);
        assert!(result.is_err());

        // Galea requires ip_address
        let result = controller.prepare_session(BoardId::Galea, &params);
        assert!(result.is_err());

        // Streaming requires ip_address, ip_port, and master_board
        let result = controller.prepare_session(BoardId::Streaming, &params);
        assert!(result.is_err());

        // PlaybackFile requires file and master_board
        let result = controller.prepare_session(BoardId::PlaybackFile, &params);
        assert!(result.is_err());
    }

    #[test]
    fn test_streaming_board_creation() {
        let controller = BoardController::new();
        let params = BoardParams::new()
            .with_ip_address("127.0.0.1")
            .with_ip_port(5000);

        // Still missing master_board
        let result = controller.prepare_session(BoardId::Streaming, &params);
        assert!(result.is_err());
    }

    #[test]
    fn test_playback_board_missing_params() {
        let controller = BoardController::new();

        // Missing master_board should fail
        let params = BoardParams::new().with_file("test.csv");
        let result = controller.prepare_session(BoardId::PlaybackFile, &params);
        assert!(result.is_err());

        // Missing file should fail
        let mut params = BoardParams::new();
        params.master_board = Some(BoardId::Synthetic);
        let result = controller.prepare_session(BoardId::PlaybackFile, &params);
        assert!(result.is_err());
    }

    #[test]
    fn test_all_boards_supported() {
        let controller = BoardController::new();
        let params = BoardParams::new();

        // BrainBit should work (all boards are now supported)
        let result = controller.prepare_session(BoardId::BrainBit, &params);
        assert!(result.is_ok());
        controller.release_session(BoardId::BrainBit, &params).unwrap();

        // Unicorn should also work
        let result = controller.prepare_session(BoardId::Unicorn, &params);
        assert!(result.is_ok());
        controller.release_session(BoardId::Unicorn, &params).unwrap();
    }

    #[test]
    fn test_get_board_description() {
        let controller = BoardController::new();
        let params = BoardParams::new();

        controller.prepare_session(BoardId::Synthetic, &params).unwrap();

        let desc = controller.get_board_description(BoardId::Synthetic, &params, Preset::Default).unwrap();
        assert_eq!(desc.sampling_rate, 250);
        assert!(!desc.eeg_channels.is_empty());

        controller.release_session(BoardId::Synthetic, &params).unwrap();
    }

    #[test]
    fn test_double_prepare_fails() {
        let controller = BoardController::new();
        let params = BoardParams::new();

        controller.prepare_session(BoardId::Synthetic, &params).unwrap();

        // Second prepare should fail
        let result = controller.prepare_session(BoardId::Synthetic, &params);
        assert!(result.is_err());

        controller.release_session(BoardId::Synthetic, &params).unwrap();
    }

    #[test]
    fn test_release_without_prepare_fails() {
        let controller = BoardController::new();
        let params = BoardParams::new();

        // Release without prepare should fail
        let result = controller.release_session(BoardId::Synthetic, &params);
        assert!(result.is_err());
    }
}

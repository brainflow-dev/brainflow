use ndarray::Array2;
use std::{thread, time::Duration};

use brainflow::{
    board_shim, brainflow_input_params::BrainFlowInputParamsBuilder, data_filter, BoardIds, BrainFlowPresets,
};
use ndarray::s;

fn main() {
    brainflow::board_shim::enable_dev_board_logger().unwrap();

    let board_id = BoardIds::SyntheticBoard;
    let eeg_channels = board_shim::get_eeg_channels(board_id, BrainFlowPresets::DefaultPreset).unwrap();
    println!("{:?}", eeg_channels);

    let params = BrainFlowInputParamsBuilder::default().build();
    let board = board_shim::BoardShim::new(board_id, params).unwrap();

    board.prepare_session().unwrap();
    board.start_stream(45000, "").unwrap();
    thread::sleep(Duration::from_secs(10));
    board.stop_stream().unwrap();
    let mut data = board.get_board_data(Some(500), BrainFlowPresets::DefaultPreset).unwrap();
    board.release_session().unwrap();

    let my_channel = eeg_channels[4];
    let mut my_data = data.slice_mut(s![my_channel, ..]);
    let data_my_channel = my_data.as_slice_mut().unwrap();
    let ica_data = Array2::from_shape_vec((5, 100), data_my_channel.to_vec()).unwrap();
    let _ica = data_filter::perform_ica(ica_data, 2).unwrap();
}

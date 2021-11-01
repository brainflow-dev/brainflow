use std::{thread, time::Duration};

use brainflow::{
    board_shim, brainflow_input_params::BrainFlowInputParamsBuilder, data_filter, AggOperations,
    BoardIds,
};

fn main() {
    brainflow::board_shim::enable_dev_board_logger().unwrap();
    let board_id = BoardIds::SyntheticBoard as i32;
    let eeg_channels = board_shim::get_eeg_channels(board_id).unwrap();

    let params = BrainFlowInputParamsBuilder::default().build();
    let board_id = BoardIds::SyntheticBoard as i32;
    let board = board_shim::BoardShim::new(board_id, params).unwrap();

    board.prepare_session().unwrap();
    board.start_stream(45000, "").unwrap();
    thread::sleep(Duration::from_secs(5));
    board.stop_stream().unwrap();
    let mut data = board.get_board_data(Some(128)).unwrap();
    board.release_session().unwrap();

    println!("{:?}", data[eeg_channels[0]]);
    data_filter::perform_rolling_filter(&mut data[eeg_channels[0]], 3, AggOperations::Mean as i32)
        .unwrap();
    println!("{:?}", data[eeg_channels[0]]);

    println!("{:?}", data[eeg_channels[1]]);
    data_filter::perform_wavelet_denoising(&mut data[eeg_channels[1]], "db3", 3).unwrap();
    println!("{:?}", data[eeg_channels[1]]);
}

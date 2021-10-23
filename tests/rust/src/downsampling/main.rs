use std::thread;
use std::time::Duration;

use brainflow::board_shim;
use brainflow::brainflow_input_params::BrainFlowInputParamsBuilder;
use brainflow::BoardIds;
use brainflow::data_filter;
use brainflow::AggOperations;

#[allow(unused_must_use)]
fn main() {
    brainflow::board_shim::enable_dev_board_logger().unwrap();
    let params = BrainFlowInputParamsBuilder::default()
        .build();
    let board_id = BoardIds::SyntheticBoard as i32;
    let board = board_shim::BoardShim::new(board_id, params).unwrap();
    let eeg_channels = board_shim::get_eeg_channels(board_id).unwrap();

    board.prepare_session().unwrap();
    board.start_stream(45000, "").unwrap();
    thread::sleep(Duration::from_secs(5));
    board.stop_stream().unwrap();
    let mut data = board.get_board_data(Some(128)).unwrap();
    board.release_session().unwrap();

    println!("{:?}", data[eeg_channels[0]]);
    let downsampled = data_filter::perform_downsampling(&mut data[eeg_channels[0]], 3, AggOperations::Each as i32).unwrap();
    println!("{:?}", downsampled);
}

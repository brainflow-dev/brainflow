use std::thread;
use std::time::Duration;

use brainflow::board_shim;
use brainflow::brainflow_input_params::BrainFlowInputParamsBuilder;
use brainflow::BoardIds;
use brainflow::data_filter;
use brainflow::FilterTypes;

#[allow(unused_must_use)]
fn main() {
    brainflow::board_shim::enable_dev_board_logger().unwrap();
    let params = BrainFlowInputParamsBuilder::default()
        .build();
    let board_id = BoardIds::SyntheticBoard as i32;
    let board = board_shim::BoardShim::new(board_id, params).unwrap();
    let sampling_rate = board_shim::get_sampling_rate(board_id).unwrap();
    let eeg_channels = board_shim::get_eeg_channels(board_id).unwrap();
    println!("{:?}", eeg_channels);

    board.prepare_session().unwrap();
    board.start_stream(45000, "").unwrap();
    thread::sleep(Duration::from_secs(5));
    board.stop_stream().unwrap();
    let mut data = board.get_board_data(Some(10)).unwrap();
    board.release_session().unwrap();

    println!("{:?}", data[eeg_channels[0]]);

    data_filter::perform_lowpass(&mut data[eeg_channels[0]], sampling_rate, 13.0, 3, FilterTypes::Bessel as i32, 0.0);

}

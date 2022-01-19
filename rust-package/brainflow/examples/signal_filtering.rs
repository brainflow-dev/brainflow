use std::{thread, time::Duration};

use brainflow::{
    board_shim, brainflow_input_params::BrainFlowInputParamsBuilder, data_filter, BoardIds,
    FilterTypes,
};
use ndarray::s;

fn main() {
    brainflow::board_shim::enable_dev_board_logger().unwrap();
    let board_id = BoardIds::SyntheticBoard;
    let sampling_rate = board_shim::get_sampling_rate(board_id).unwrap();
    let eeg_channels = board_shim::get_eeg_channels(board_id).unwrap();

    let params = BrainFlowInputParamsBuilder::default().build();
    let board = board_shim::BoardShim::new(board_id, params).unwrap();

    board.prepare_session().unwrap();
    board.start_stream(45000, "").unwrap();
    thread::sleep(Duration::from_secs(5));
    board.stop_stream().unwrap();
    let mut data = board.get_board_data(Some(10)).unwrap();
    board.release_session().unwrap();

    dbg!(&data);
    println!("{:?}", data.slice(s![eeg_channels[0], ..]));

    data_filter::perform_lowpass(
        data.slice_mut(s![eeg_channels[0], ..])
            .as_slice_mut()
            .unwrap(),
        sampling_rate,
        13.0,
        3,
        FilterTypes::Bessel,
        0.0,
    )
    .unwrap();

    println!("{:?}", data.slice(s![eeg_channels[0], ..]));
}

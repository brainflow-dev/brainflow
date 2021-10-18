use std::thread;
use std::time::Duration;

use brainflow::board_shim;
use brainflow::brainflow_input_params::BrainFlowInputParamsBuilder;
use brainflow::BoardIds;
use brainflow::data_filter;
use brainflow::WindowFunctions;

#[allow(unused_must_use)]
fn main() {
    brainflow::board_shim::enable_dev_board_logger().unwrap();
    let params = BrainFlowInputParamsBuilder::default()
        .build();
    let board_id = BoardIds::SyntheticBoard as i32;
    let board = board_shim::BoardShim::new(board_id, params).unwrap();
    let eeg_channels = board_shim::get_eeg_channels(board_id).unwrap();
    println!("{:?}", eeg_channels);

    board.prepare_session().unwrap();
    board.start_stream(45000, "").unwrap();
    thread::sleep(Duration::from_secs(5));
    board.stop_stream().unwrap();
    let mut data = board.get_board_data(Some(64)).unwrap();
    let data_len = data[0].len();
    board.release_session().unwrap();

    println!("{:?}", data[eeg_channels[0]]);
    let fft_data = data_filter::perform_fft(&mut data[eeg_channels[0]], WindowFunctions::BlackmanHarris as i32).unwrap();
    let restored_fft = data_filter::perform_ifft(&fft_data, data_len).unwrap();
    println!("{:?}", restored_fft);

    println!("{:?}", data[eeg_channels[1]]);
    let wavelet_data = data_filter::perform_wavelet_transform(&mut data[eeg_channels[1]], "db3", 3).unwrap();
    let restored_wavelet = data_filter::perform_inverse_wavelet_transform(wavelet_data).unwrap();
    println!("{:?}", restored_wavelet);
}

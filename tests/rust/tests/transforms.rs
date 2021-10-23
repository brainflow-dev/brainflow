#[macro_use]
extern crate approx;

use brainflow::board_shim;
use brainflow::data_filter;
use brainflow::BoardIds;
use brainflow::WindowFunctions;

mod common;

#[test]
fn wavelet_inverse_transform_equals_input_data() {
    let board_id = BoardIds::SyntheticBoard as i32;
    let eeg_channels = board_shim::get_eeg_channels(board_id).unwrap();
    println!("{:?}", eeg_channels);

    let mut data = common::board_data(Some(64));
    let data_len = data[0].len();

    let fft_data = data_filter::perform_fft(
        &mut data[eeg_channels[0]],
        WindowFunctions::BlackmanHarris as i32,
    )
    .unwrap();
    let restored_fft = data_filter::perform_ifft(&fft_data, data_len).unwrap();
    println!("{:?}", restored_fft);

    println!("{:?}", data[eeg_channels[1]]);
    let wavelet_data =
        data_filter::perform_wavelet_transform(&mut data[eeg_channels[1]], "db3", 3).unwrap();
    let restored_wavelet = data_filter::perform_inverse_wavelet_transform(wavelet_data).unwrap();
    println!("{:?}", restored_wavelet);
    for (d, r) in data[eeg_channels[1]].iter().zip(restored_wavelet) {
        assert_relative_eq!(*d, r, max_relative = 1e-14);
    }
}

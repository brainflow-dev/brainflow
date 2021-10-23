use brainflow::board_shim;
use brainflow::data_filter;
use brainflow::AggOperations;
use brainflow::BoardIds;

mod common;

#[test]
fn denoising() {
    let board_id = BoardIds::SyntheticBoard as i32;
    let eeg_channels = board_shim::get_eeg_channels(board_id).unwrap();

    let mut data = common::board_data(Some(128));

    println!("{:?}", data[eeg_channels[0]]);
    data_filter::perform_rolling_filter(&mut data[eeg_channels[0]], 3, AggOperations::Mean as i32)
        .unwrap();
    println!("{:?}", data[eeg_channels[0]]);

    println!("{:?}", data[eeg_channels[1]]);
    data_filter::perform_wavelet_denoising(&mut data[eeg_channels[1]], "db3", 3).unwrap();
    println!("{:?}", data[eeg_channels[1]]);
}

use brainflow::board_shim;
use brainflow::data_filter;
use brainflow::BoardIds;
use brainflow::FilterTypes;

mod common;

#[test]
fn signal_filtering() {
    let board_id = BoardIds::SyntheticBoard as i32;
    let sampling_rate = board_shim::get_sampling_rate(board_id).unwrap();
    let eeg_channels = board_shim::get_eeg_channels(board_id).unwrap();

    let mut data = common::board_data(Some(10));

    println!("{:?}", data[eeg_channels[0]]);

    data_filter::perform_lowpass(
        &mut data[eeg_channels[0]],
        sampling_rate,
        13.0,
        3,
        FilterTypes::Bessel as i32,
        0.0,
    )
    .unwrap();

    println!("{:?}", data[eeg_channels[0]]);
}

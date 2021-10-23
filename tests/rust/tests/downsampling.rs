use brainflow::board_shim;
use brainflow::data_filter;
use brainflow::AggOperations;
use brainflow::BoardIds;

mod common;

#[test]
fn downsampling() {
    let board_id = BoardIds::SyntheticBoard as i32;
    let eeg_channels = board_shim::get_eeg_channels(board_id).unwrap();

    let mut data = common::board_data(Some(128));

    dbg!(&data[eeg_channels[0]]);
    let downsampled = data_filter::perform_downsampling(
        &mut data[eeg_channels[0]],
        3,
        AggOperations::Each as i32,
    )
    .unwrap();
    dbg!(&downsampled);
    assert_eq!(downsampled.len(), data[eeg_channels[0]].len() / 3);
}

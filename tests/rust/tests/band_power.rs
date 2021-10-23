use approx::assert_relative_eq;
use brainflow::board_shim;
use brainflow::data_filter;
use brainflow::BoardIds;
use brainflow::DetrendOperations;
use brainflow::WindowFunctions;

mod common;

#[test]
fn bandpower() {
    let board_id = BoardIds::SyntheticBoard as i32;
    let eeg_channels = board_shim::get_eeg_channels(board_id).unwrap();
    let sampling_rate = board_shim::get_sampling_rate(board_id).unwrap();
    let nfft = data_filter::get_nearest_power_of_two(sampling_rate).unwrap();

    let mut data = common::board_data(None);

    data_filter::detrend(&mut data[eeg_channels[1]], DetrendOperations::Linear as i32).unwrap();
    let mut psd = data_filter::get_psd_welch(
        &mut data[eeg_channels[1]],
        nfft,
        nfft / 2,
        sampling_rate,
        WindowFunctions::BlackmanHarris as i32,
    )
    .unwrap();
    let band_power_alpha = data_filter::get_band_power(&mut psd, 7.0, 13.0).unwrap();
    let band_power_beta = data_filter::get_band_power(&mut psd, 14.0, 30.0).unwrap();
    assert_relative_eq!(band_power_alpha / band_power_beta, 2500.0, epsilon = 1000.0);
}

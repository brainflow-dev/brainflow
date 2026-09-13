BoardShim.set_log_file('brainflow.log');
BoardShim.enable_dev_board_logger();

params = BrainFlowInputParams();
board_shim = BoardShim(int32(BoardIds.SYNTHETIC_BOARD), params);
preset = int32(BrainFlowPresets.DEFAULT_PRESET);
board_shim.prepare_session();
board_shim.start_stream(45000, '');
pause(5);
board_shim.stop_stream();
data = board_shim.get_board_data(64, preset);
board_shim.release_session();

eeg_channels = BoardShim.get_eeg_channels(int32(BoardIds.SYNTHETIC_BOARD), preset);
% Re-reference every EEG channel using the sample-wise mean of the first two EEG channels.
% DataFilter.reference returns a referenced copy of the matrix.
if length(eeg_channels) >= 2
    data = DataFilter.reference(data, eeg_channels, eeg_channels(1:2));
end

% apply downsampling to the first eeg channel %
first_eeg_channel = eeg_channels(1);
original_data = data(first_eeg_channel, :);
downsampled_data = DataFilter.perform_downsampling(original_data, 3, int32(AggOperations.MEAN));

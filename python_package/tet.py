import time
import numpy as np
from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds, BrainFlowError, LogLevels
import faulthandler
import csv
faulthandler.enable()

def calculate_rms(signal):
    return np.sqrt(np.mean(np.square(signal)))

def calculate_peak_to_peak(signal):
    return np.max(signal) - np.min(signal)

def test_my_board():
    params = BrainFlowInputParams()
    params.serial_port = 'COM4'  # Set your port here

    time_len = 10 # seconds
    try:
        board = BoardShim(BoardIds.CERELOG_X8_BOARD, params)
        BoardShim.enable_dev_board_logger()
        BoardShim.set_log_level(LogLevels.LEVEL_DEBUG.value)
        BoardShim.set_log_file('test_1.log')
        sample_rate = BoardShim.get_sampling_rate(BoardIds.CERELOG_X8_BOARD)
        eeg_channels = BoardShim.get_eeg_channels(BoardIds.CERELOG_X8_BOARD)
        print(f"Sample rate  : {sample_rate} SPS")
        print(f"EEG Channels : {eeg_channels}")

        board.prepare_session()
        print("✓ Session prepared successfully")

        board.start_stream()
        print("... Stream started for {} seconds".format(time_len))

        time.sleep(time_len)  # Collect some data
        #print("after sleep call")
        board.stop_stream()
        print("Stream time completed")
        data = board.get_board_data()

        print(f"Data shape: {data.shape}")
        #print("First few data points:")
        #print(data[:5, :10] if data.size > 0 else "No data collected")
        print(f"✓ Got {data.shape[1]} samples")

        # Calculate RMS for each EEG channel
        if data.size > 0:
            for ch in eeg_channels:
                ch_data = data[ch]
                rms = calculate_rms(ch_data)
                print(f"RMS of EEG channel {ch}: {rms:.4f} V")

        # Calculate and print average Vpp of the channels
        vpp_values = []
        for ch in eeg_channels:
            ch_data = data[ch]
            ptp = calculate_peak_to_peak(ch_data)
            if ptp >= 0.01:  # Exclude channels with Vpp less than 0.01
                vpp_values.append(ptp)
        if vpp_values:
            avg_vpp = np.mean(vpp_values)
            print(f"Average Vpp of EEG channels: {avg_vpp:.4f} V")
        else:
            print("No channels with Vpp >= 0.01 to calculate average Vpp.")
        # Write data to CSV
        with open('data.csv', mode='w', newline='') as file:
            writer = csv.writer(file)
            writer.writerow(['Time'] + [f"Channel {ch}" for ch in eeg_channels])
            for i in range(data.shape[1]):
                writer.writerow([i] + [f"{data[ch][i]:.4f}" for ch in eeg_channels])

        board.release_session()
        print("✓ Done!")

    except BrainFlowError as e:
        print(f"✗ Error: {e}")

if __name__ == "__main__":
    test_my_board()
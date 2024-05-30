import logging
import socket
import numpy as np
from brainflow.data_filter import DataFilter, FilterTypes, NoiseTypes

def analyze_frequency_bands(data, sampling_rate):
    try:
        num_channels = 8
        reshaped_data = data.reshape((num_channels, -1))

        # Perform band power analysis
        bands = DataFilter.get_avg_band_powers(reshaped_data, np.arange(num_channels), sampling_rate, True)
        avgs = bands[0]
        return avgs
    except Exception as e:
        logging.error(f"Error analyzing frequency bands: {e}")
        return None

def process_eeg_data(eeg_data):
    if eeg_data.size == 0:
        logging.error("Received empty EEG data array")
        return None

    num_channels = 8
    if eeg_data.size % num_channels != 0:
        logging.error(f"Cannot reshape array of size {eeg_data.size} into shape ({num_channels}, -1)")
        return None

    eeg_data = eeg_data.reshape((num_channels, -1))

    sampling_rate = 250
    valid_data = True
    for channel_data in eeg_data:
        if np.isnan(channel_data).any() or not np.isfinite(channel_data).all():
            logging.error("Invalid channel data detected, skipping this chunk")
            valid_data = False
            break

        DataFilter.perform_bandpass(channel_data, sampling_rate, 0.5, 50.0, 4, FilterTypes.BUTTERWORTH.value, 0)
        DataFilter.remove_environmental_noise(channel_data, sampling_rate, NoiseTypes.FIFTY.value)

    if not valid_data:
        return None

    eeg_data = eeg_data.flatten()

    if len(eeg_data) < sampling_rate:
        logging.error("Not enough data points for frequency analysis")
        return None

    return analyze_frequency_bands(eeg_data, sampling_rate)

def main():
    logging.basicConfig(level=logging.INFO)
    server_ip = '192.168.1.175'
    port = 64677
    buffer = bytearray()

    try:
        logging.info(f"Connecting to {server_ip}:{port}")
        with socket.create_connection((server_ip, port)) as conn:
            while True:
                data = conn.recv(10000)  # Adjust the buffer size if needed
                if not data:
                    break
                buffer.extend(data)

                chunk_size = 8 * 250  # Number of channels * number of samples per chunk
                while len(buffer) >= chunk_size * 8:  # 8 bytes per double value
                    chunk = buffer[:chunk_size * 8]
                    buffer = buffer[chunk_size * 8:]
                    eeg_data = np.frombuffer(chunk, dtype=np.float64)
                    logging.info(f"Received EEG data chunk: {eeg_data}")

                    frequency_bands = process_eeg_data(eeg_data)
                    if frequency_bands is not None:
                        logging.info(f"Processed frequency bands: {frequency_bands}")

    except Exception as e:
        logging.error(f"Error: {e}")

if __name__ == "__main__":
    main()

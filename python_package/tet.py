import time
from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds, BrainFlowError
import numpy as np


def test_my_board():
    params = BrainFlowInputParams()
    params.serial_port = 'COM4'  # Your port
    
    try:
        board = BoardShim(BoardIds.CERELOG_X8_BOARD, params)
        BoardShim.enable_dev_board_logger()
        BoardShim.set_log_file('brainflow.log')
        board.prepare_session()
        print("✓ Session prepared successfully")
        
        board.start_stream()
        print("✓ Stream started")
        
        time.sleep(2)  # Collect some data
        
        board.stop_stream()
        data = board.get_board_data()

        print(f"Data shape: {data.shape}")
        print(f"Sample rate info: {BoardShim.get_sampling_rate(BoardIds.CERELOG_X8_BOARD)}")
        print(f"EEG channels: {BoardShim.get_eeg_channels(BoardIds.CERELOG_X8_BOARD)}")
        print("First few data points:")
        print(data[:5, :10] if data.size > 0 else "No data collected")
        print(f"✓ Got {data.shape[1]} samples")
        
        board.release_session()
        print("✓ All good!")
        
    except BrainFlowError as e:
        print(f"✗ Error: {e}")

if __name__ == "__main__":
    test_my_board()
import time
from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds, BrainFlowError

def test_my_board():
    params = BrainFlowInputParams()
    params.serial_port = 'COM3'  # Your port
    
    try:
        board = BoardShim(BoardIds.CERELOG_X8_BOARD, params)
        board.prepare_session()
        print("✓ Session prepared successfully")
        
        board.start_stream()
        print("✓ Stream started")
        
        time.sleep(2)  # Collect some data
        
        data = board.get_board_data()
        print(f"✓ Got {data.shape[1]} samples")
        
        board.stop_stream()
        board.release_session()
        print("✓ All good!")
        
    except BrainFlowError as e:
        print(f"✗ Error: {e}")

if __name__ == "__main__":
    test_my_board()
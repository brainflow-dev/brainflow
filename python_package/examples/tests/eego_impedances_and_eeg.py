import time

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds

if __name__ == '__main__':
    params = BrainFlowInputParams()
    board = BoardShim(BoardIds.ANT_NEURO_EE_411_BOARD, params)  # 8 channel amplifier
    board.prepare_session()

    # Get amplifier info
    print(f"get_type: {board.config_board('get_type')}")
    print(f"get_firmware_version: {board.config_board('get_firmware_version')}")
    print(f"get_serial_number: {board.config_board('get_serial_number')}")
    print(f"get_sampling_rates_available: {board.config_board('get_sampling_rates_available')}")
    print(f"get_reference_ranges_available: {board.config_board('get_reference_ranges_available')}")
    print(f"get_bipolar_ranges_available: {board.config_board('get_bipolar_ranges_available')}")
    print(f"get_power_state: {board.config_board('get_power_state')}")

    # Get impedance data
    board.config_board('impedance_mode:1')
    board.start_stream()
    for i in range(5):
        time.sleep(1)
        data = board.get_board_data()  # get all data and remove it from internal buffer
        print(f'{data.shape[0]} channels x {data.shape[1]} samples')
    board.stop_stream()
    
    # Get EEG data
    board.config_board('impedance_mode:0')
    board.start_stream()
    for i in range(3):
        time.sleep(1)
        data = board.get_board_data()  # get all data and remove it from internal buffer
        print(f'{data.shape[0]} channels x {data.shape[1]} samples')
    board.stop_stream()

    board.release_session()


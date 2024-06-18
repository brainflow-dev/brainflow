import time

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds, BrainFlowPresets
from brainflow.data_filter import DataFilter

def main():
    BoardShim.enable_dev_board_logger()
    DataFilter.enable_dev_data_logger()

    params = BrainFlowInputParams()

    board = BoardShim(BoardIds.MUSE_2_BOARD, params)
    board.prepare_session()
    board.config_board('p50')
    board.start_stream()
    time.sleep(200)
    data = board.get_board_data(preset=BrainFlowPresets.ANCILLARY_PRESET)
    board.stop_stream()
    board.release_session()

    print(data)

    ppg_channels = BoardShim.get_ppg_channels(BoardIds.MUSE_2_BOARD,  BrainFlowPresets.ANCILLARY_PRESET)
    sampling_rate = BoardShim.get_sampling_rate(BoardIds.MUSE_2_BOARD, BrainFlowPresets.ANCILLARY_PRESET)
    ppg_ir = data[ppg_channels[1]]
    ppg_red = data[ppg_channels[0]] 
    oxygen_level = DataFilter.get_oxygen_level(ppg_ir, ppg_red, sampling_rate)
    print(oxygen_level)
    # 8192 and longer recorindg is recommended, in this test use 2048
    heart_rate = DataFilter.get_heart_rate(ppg_ir, ppg_red, sampling_rate, 2048) 
    print(heart_rate)


if __name__ == "__main__":
    main()

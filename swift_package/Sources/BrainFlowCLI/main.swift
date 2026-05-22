import BrainFlow
import Foundation

let boardId = BoardIds.SYNTHETIC_BOARD
var params = BrainFlowInputParams()

do {
    try BoardShim.enable_board_logger()
    let board = try BoardShim(board_id: boardId, input_params: params)
    try board.prepare_session()
    try board.start_stream(buffer_size: 45000)
    Thread.sleep(forTimeInterval: 2.0)
    try board.stop_stream()
    let data = try board.get_board_data()
    try board.release_session()

    let rows = data.count
    let cols = data.first?.count ?? 0
    let samplingRate = try BoardShim.get_sampling_rate(board_id: boardId.rawValue)
    let eegChannels = try BoardShim.get_eeg_channels(board_id: boardId.rawValue)

    print("BrainFlow Swift synthetic board sample")
    print("board_id=\(boardId.rawValue)")
    print("sampling_rate=\(samplingRate)")
    print("rows=\(rows) cols=\(cols)")
    print("eeg_channels=\(eegChannels)")
} catch {
    fputs("BrainFlow CLI failed: \(error)\n", stderr)
    exit(1)
}

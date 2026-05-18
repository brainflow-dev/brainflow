import BrainFlow
import Foundation

let board = try BoardShim(board_id: BoardIds.SYNTHETIC_BOARD)
try board.prepare_session()
try board.start_stream(buffer_size: 45000)
try board.insert_marker(1.0)
Thread.sleep(forTimeInterval: 1.0)
try board.stop_stream()
let data = try board.get_board_data()
try board.release_session()

let markerChannel = try BoardShim.get_marker_channel(board_id: BoardIds.SYNTHETIC_BOARD.rawValue)
print("Marker channel: \(markerChannel)")
print("Samples: \(data[markerChannel].count)")

import BrainFlow
import Foundation

let board = try BoardShim(board_id: BoardIds.SYNTHETIC_BOARD)
try board.prepare_session()
try board.start_stream(buffer_size: 45000)
Thread.sleep(forTimeInterval: 5.0)
try board.stop_stream()
let data = try board.get_board_data()
try board.release_session()

print("Rows: \(data.count)")
print("Samples: \(data.first?.count ?? 0)")

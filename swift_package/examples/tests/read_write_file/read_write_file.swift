import BrainFlow
import Foundation

let board = try BoardShim(board_id: BoardIds.SYNTHETIC_BOARD)
try board.prepare_session()
try board.start_stream(buffer_size: 45000)
Thread.sleep(forTimeInterval: 2.0)
try board.stop_stream()
let data = try board.get_board_data()
try board.release_session()

let fileName = NSTemporaryDirectory() + "/brainflow_swift.csv"
try DataFilter.write_file(data: data, file_name: fileName, file_mode: "w")
let restored = try DataFilter.read_file(fileName)
print("Original: \(data.count)x\(data.first?.count ?? 0)")
print("Restored: \(restored.count)x\(restored.first?.count ?? 0)")

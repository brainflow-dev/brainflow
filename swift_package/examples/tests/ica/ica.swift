import BrainFlow
import Foundation

let board = try BoardShim(board_id: BoardIds.SYNTHETIC_BOARD)
try board.prepare_session()
try board.start_stream(buffer_size: 45000)
Thread.sleep(forTimeInterval: 5.0)
try board.stop_stream()
let data = try board.get_board_data(500)
try board.release_session()

let channels = Array(try BoardShim.get_eeg_channels(board_id: BoardIds.SYNTHETIC_BOARD.rawValue).prefix(4))
let ica = try DataFilter.perform_ica(data: data, num_components: 2, channels: channels)
print("W: \(ica.w.count)x\(ica.w.first?.count ?? 0)")
print("S: \(ica.s.count)x\(ica.s.first?.count ?? 0)")

import BrainFlow
import Foundation

let board = try BoardShim(board_id: BoardIds.SYNTHETIC_BOARD)
try board.prepare_session()
try board.start_stream(buffer_size: 45000)
Thread.sleep(forTimeInterval: 5.0)
try board.stop_stream()
let data = try board.get_board_data()
try board.release_session()

let channels = try BoardShim.get_eeg_channels(board_id: BoardIds.SYNTHETIC_BOARD.rawValue)
let samplingRate = try BoardShim.get_sampling_rate(board_id: BoardIds.SYNTHETIC_BOARD.rawValue)
let bandPowers = try DataFilter.get_avg_band_powers(data: data, channels: channels, sampling_rate: samplingRate, apply_filter: true)

var params = BrainFlowModelParams(metric: BrainFlowMetrics.MINDFULNESS, classifier: BrainFlowClassifiers.DEFAULT_CLASSIFIER)
let model = try MLModel(params: params)
try model.prepare()
let prediction = try model.predict(input_data: bandPowers.average)
try model.release()
print(prediction)

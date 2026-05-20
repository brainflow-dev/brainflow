import BrainFlow
import Foundation

public struct SyntheticBoardData {
    public let boardId: BoardIds
    public let data: [[Double]]
    public let eegChannels: [Int]
    public let samplingRate: Int

    public func firstEEGChannel() throws -> [Double] {
        guard let channel = eegChannels.first, channel >= 0, channel < data.count else {
            throw BrainFlowError("No EEG channel found in synthetic board data", BrainFlowExitCodes.GENERAL_ERROR.rawValue)
        }
        return data[channel]
    }
}

public enum SyntheticBoardDataReader {
    public static func read(seconds: TimeInterval = 5.0, maxSamples: Int? = nil) throws -> SyntheticBoardData {
        let boardId = BoardIds.SYNTHETIC_BOARD
        let board = try BoardShim(board_id: boardId)
        try board.prepare_session()

        do {
            try board.start_stream(buffer_size: 45_000)
            Thread.sleep(forTimeInterval: seconds)
            try board.stop_stream()
            let data = try board.get_board_data(maxSamples)
            try board.release_session()

            return SyntheticBoardData(
                boardId: boardId,
                data: data,
                eegChannels: try BoardShim.get_eeg_channels(board_id: boardId),
                samplingRate: try BoardShim.get_sampling_rate(board_id: boardId)
            )
        } catch {
            try? board.release_session()
            throw error
        }
    }
}

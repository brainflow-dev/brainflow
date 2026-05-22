import Foundation

public enum BrainFlowExitCodes: Int, CaseIterable, Sendable {
    case STATUS_OK = 0
    case PORT_ALREADY_OPEN_ERROR = 1
    case UNABLE_TO_OPEN_PORT_ERROR = 2
    case SET_PORT_ERROR = 3
    case BOARD_WRITE_ERROR = 4
    case INCOMMING_MSG_ERROR = 5
    case INITIAL_MSG_ERROR = 6
    case BOARD_NOT_READY_ERROR = 7
    case STREAM_ALREADY_RUN_ERROR = 8
    case INVALID_BUFFER_SIZE_ERROR = 9
    case STREAM_THREAD_ERROR = 10
    case STREAM_THREAD_IS_NOT_RUNNING = 11
    case EMPTY_BUFFER_ERROR = 12
    case INVALID_ARGUMENTS_ERROR = 13
    case UNSUPPORTED_BOARD_ERROR = 14
    case BOARD_NOT_CREATED_ERROR = 15
    case ANOTHER_BOARD_IS_CREATED_ERROR = 16
    case GENERAL_ERROR = 17
    case SYNC_TIMEOUT_ERROR = 18
    case JSON_NOT_FOUND_ERROR = 19
    case NO_SUCH_DATA_IN_JSON_ERROR = 20
    case CLASSIFIER_IS_NOT_PREPARED_ERROR = 21
    case ANOTHER_CLASSIFIER_IS_PREPARED_ERROR = 22
    case UNSUPPORTED_CLASSIFIER_AND_METRIC_COMBINATION_ERROR = 23

    public var code: Int { rawValue }
}

public struct BrainFlowError: Error, LocalizedError, CustomStringConvertible, Sendable {
    public let message: String
    public let exit_code: Int

    public init(_ message: String, _ exit_code: Int) {
        self.message = message
        self.exit_code = exit_code
    }

    public var errorDescription: String? {
        "\(message), exit code: \(exit_code)"
    }

    public var description: String {
        errorDescription ?? message
    }
}

@inline(__always)
func checkBrainFlowExitCode(_ exitCode: CInt, _ message: String) throws {
    guard exitCode == CInt(BrainFlowExitCodes.STATUS_OK.rawValue) else {
        throw BrainFlowError(message, Int(exitCode))
    }
}

func invalidArguments(_ message: String) -> BrainFlowError {
    BrainFlowError(message, BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.rawValue)
}

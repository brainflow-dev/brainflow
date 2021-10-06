//
//  BrainFlowConstants.swift
//  a Swift binding for BrainFlow's enumeration types
//
//  Created by Scott Miller for Aeris Rising, LLC on 8/23/21.
//

enum BrainFlowExitCodes : Int32, Error {
    case UNKNOWN_CODE = -1
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
}

enum BoardIds : Int32, CaseIterable {
    case PLAYBACK_FILE_BOARD = -3
    case STREAMING_BOARD = -2
    case SYNTHETIC_BOARD = -1
    case CYTON_BOARD = 0
    case GANGLION_BOARD = 1
    case CYTON_DAISY_BOARD = 2
    case GALEA_BOARD = 3
    case GANGLION_WIFI_BOARD = 4
    case CYTON_WIFI_BOARD = 5
    case CYTON_DAISY_WIFI_BOARD = 6
    case BRAINBIT_BOARD = 7
    case UNICORN_BOARD = 8
    case CALLIBRI_EEG_BOARD = 9
    case CALLIBRI_EMG_BOARD = 10
    case CALLIBRI_ECG_BOARD = 11
    case FASCIA_BOARD = 12
    case NOTION_1_BOARD = 13
    case NOTION_2_BOARD = 14
    case IRONBCI_BOARD = 15
    case GFORCE_PRO_BOARD = 16
    case FREEEEG32_BOARD = 17
    case BRAINBIT_BLED_BOARD = 18
    case GFORCE_DUAL_BOARD = 19
    case GALEA_SERIAL_BOARD = 20
    case MUSE_S_BLED_BOARD = 21
    case MUSE_2_BLED_BOARD = 22
    case CROWN_BOARD = 23
    case ANT_NEURO_EE_410_BOARD = 24
    case ANT_NEURO_EE_411_BOARD = 25
    case ANT_NEURO_EE_430_BOARD = 26
    case ANT_NEURO_EE_211_BOARD = 27
    case ANT_NEURO_EE_212_BOARD = 28
    case ANT_NEURO_EE_213_BOARD = 29
    case ANT_NEURO_EE_214_BOARD = 30
    case ANT_NEURO_EE_215_BOARD = 31
    case ANT_NEURO_EE_221_BOARD = 32
    case ANT_NEURO_EE_222_BOARD = 33
    case ANT_NEURO_EE_223_BOARD = 34
    case ANT_NEURO_EE_224_BOARD = 35
    case ANT_NEURO_EE_225_BOARD = 36
    case ENOPHONE_BOARD = 37
    // use it to iterate
    //case FIRST = PLAYBACK_FILE_BOARD
    //case LAST = ENOPHONE_BOARD
}

enum IpProtocolType : Int32 {
    case NONE = 0
    case UDP = 1
    case TCP = 2
}

enum FilterTypes : Int32 {
    case BUTTERWORTH = 0
    case CHEBYSHEV_TYPE_1 = 1
    case BESSEL = 2
}

enum AggOperations : Int32 {
    case MEAN = 0
    case MEDIAN = 1
    case EACH = 2
}

enum WindowFunctions : Int32, CaseIterable {
    case NO_WINDOW = 0
    case HANNING = 1
    case HAMMING = 2
    case BLACKMAN_HARRIS = 3
}

enum DetrendOperations : Int32 {
    case NONE = 0
    case CONSTANT = 1
    case LINEAR = 2
}

enum BrainFlowMetrics : Int32, Encodable {
    case RELAXATION = 0
    case CONCENTRATION = 1
}

enum BrainFlowClassifiers : Int32, Encodable {
    case REGRESSION = 0
    case KNN = 1
    case SVM = 2
    case LDA = 3
}

/// LogLevels enum to store all possible log levels
enum LogLevels : Int32 {
    case LEVEL_TRACE = 0    /// TRACE
    case LEVEL_DEBUG = 1    /// DEBUG
    case LEVEL_INFO = 2     /// INFO
    case LEVEL_WARN = 3     /// WARN
    case LEVEL_ERROR = 4    /// ERROR
    case LEVEL_CRITICAL = 5 /// CRITICAL
    case LEVEL_OFF = 6       // OFF
}

enum NoiseTypes : Int32 {
    case FIFTY = 0
    case SIXTY = 1
}

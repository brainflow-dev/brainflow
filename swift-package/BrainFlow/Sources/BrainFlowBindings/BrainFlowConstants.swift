//
//  BrainFlowConstants.swift
//  a Swift binding for BrainFlow's enumeration types
//
//  Created by Scott Miller for Aeris Rising, LLC on 8/23/21.
//

import Darwin

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

enum BoardIds : Int32, CaseIterable, Equatable {
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
    
    var name: String {
        get { String(describing: self) }
    }
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
    case MINDFULNESS = 0  
    case RESTFULNESS = 1
    case USER_DEFINED = 2
}

enum BrainFlowClassifiers : Int32, Encodable {
    case DEFAULT_CLASSIFIER = 0
    case DYN_LIB_CLASSIFIER = 1
    case ONNX_CLASSIFIER = 2
}

enum LogLevels : Int32 {
    case LEVEL_TRACE = 0 
    case LEVEL_DEBUG = 1
    case LEVEL_INFO = 2
    case LEVEL_WARN = 3
    case LEVEL_ERROR = 4
    case LEVEL_CRITICAL = 5
    case LEVEL_OFF = 6
}

enum NoiseTypes : Int32 {
    case FIFTY = 0
    case SIXTY = 1
    case FIFTY_AND_SIXTY = 2
}
    
enum WaveletDenoisingTypes : Int32 {
    case VISUSHRINK = 0
    case SURESHRINK = 1
}

enum ThresholdTypes : Int32 {
    case SOFT = 0
    case HARD = 1
}

enum WaveletExtensionTypes : Int32 {
    case SYMMETRIC = 0
    case PERIODIC = 1
}

enum NoiseEstimationLevelTypes: Int32 {
    case FIRST_LEVEL = 0
    case ALL_LEVELS = 1
}

enum WaveletTypes: Int32 {
    case HAAR = 0
    case DB1 = 1
    case DB2 = 2
    case DB3 = 3
    case DB4 = 4
    case DB5 = 5
    case DB6 = 6
    case DB7 = 7
    case DB8 = 8
    case DB9 = 9
    case DB10 = 10
    case DB11 = 11
    case DB12 = 12
    case DB13 = 13
    case DB14 = 14
    case DB15 = 15
    case BIOR1_1 = 16
    case BIOR1_3 = 17
    case BIOR1_5 = 18
    case BIOR2_2 = 19
    case BIOR2_4 = 20
    case BIOR2_6 = 21
    case BIOR2_8 = 22
    case BIOR3_1 = 23
    case BIOR3_3 = 24
    case BIOR3_5 = 25
    case BIOR3_7 = 26
    case BIOR3_9 = 27
    case BIOR4_4 = 28
    case BIOR5_5 = 29
    case BIOR6_8 = 30
    case COIF1 = 31
    case COIF2 = 32
    case COIF3 = 33
    case COIF4 = 34
    case COIF5 = 35
    case SYM2 = 36
    case SYM3 = 37
    case SYM4 = 38
    case SYM5 = 39
    case SYM6 = 40
    case SYM7 = 41
    case SYM8 = 42
    case SYM9 = 43
    case SYM10 = 44
}


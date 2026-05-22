public enum BoardIds: Int, CaseIterable, Sendable {
    case NO_BOARD = -100
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
    case NOTION_1_BOARD = 13
    case NOTION_2_BOARD = 14
    case GFORCE_PRO_BOARD = 16
    case FREEEEG32_BOARD = 17
    case BRAINBIT_BLED_BOARD = 18
    case GFORCE_DUAL_BOARD = 19
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
    case MUSE_2_BOARD = 38
    case MUSE_S_BOARD = 39
    case BRAINALIVE_BOARD = 40
    case MUSE_2016_BOARD = 41
    case MUSE_2016_BLED_BOARD = 42
    case EXPLORE_4_CHAN_BOARD = 44
    case EXPLORE_8_CHAN_BOARD = 45
    case GANGLION_NATIVE_BOARD = 46
    case EMOTIBIT_BOARD = 47
    case NTL_WIFI_BOARD = 50
    case ANT_NEURO_EE_511_BOARD = 51
    case FREEEEG128_BOARD = 52
    case AAVAA_V3_BOARD = 53
    case EXPLORE_PLUS_8_CHAN_BOARD = 54
    case EXPLORE_PLUS_32_CHAN_BOARD = 55
    case PIEEG_BOARD = 56
    case NEUROPAWN_KNIGHT_BOARD = 57
    case SYNCHRONI_TRIO_3_CHANNELS_BOARD = 58
    case SYNCHRONI_OCTO_8_CHANNELS_BOARD = 59
    case OB5000_8_CHANNELS_BOARD = 60
    case SYNCHRONI_PENTO_8_CHANNELS_BOARD = 61
    case SYNCHRONI_UNO_1_CHANNELS_BOARD = 62
    case OB3000_24_CHANNELS_BOARD = 63
    case BIOLISTENER_BOARD = 64
    case IRONBCI_32_BOARD = 65
    case NEUROPAWN_KNIGHT_BOARD_IMU = 66
    case MUSE_S_ATHENA_BOARD = 67

    public var code: Int { rawValue }
}

public enum IpProtocolTypes: Int, CaseIterable, Sendable {
    case NO_IP_PROTOCOL = 0
    case UDP = 1
    case TCP = 2

    public var code: Int { rawValue }
}

public enum FilterTypes: Int, CaseIterable, Sendable {
    case BUTTERWORTH = 0
    case CHEBYSHEV_TYPE_1 = 1
    case BESSEL = 2
    case BUTTERWORTH_ZERO_PHASE = 3
    case CHEBYSHEV_TYPE_1_ZERO_PHASE = 4
    case BESSEL_ZERO_PHASE = 5

    public var code: Int { rawValue }
}

public enum AggOperations: Int, CaseIterable, Sendable {
    case MEAN = 0
    case MEDIAN = 1
    case EACH = 2

    public var code: Int { rawValue }
}

public enum WindowOperations: Int, CaseIterable, Sendable {
    case NO_WINDOW = 0
    case HANNING = 1
    case HAMMING = 2
    case BLACKMAN_HARRIS = 3

    public var code: Int { rawValue }
}

public enum DetrendOperations: Int, CaseIterable, Sendable {
    case NO_DETREND = 0
    case CONSTANT = 1
    case LINEAR = 2

    public var code: Int { rawValue }
}

public enum BrainFlowMetrics: Int, CaseIterable, Sendable {
    case MINDFULNESS = 0
    case RESTFULNESS = 1
    case USER_DEFINED = 2

    public var code: Int { rawValue }
}

public enum BrainFlowClassifiers: Int, CaseIterable, Sendable {
    case DEFAULT_CLASSIFIER = 0
    case DYN_LIB_CLASSIFIER = 1
    case ONNX_CLASSIFIER = 2

    public var code: Int { rawValue }
}

public enum BrainFlowPresets: Int, CaseIterable, Sendable {
    case DEFAULT_PRESET = 0
    case AUXILIARY_PRESET = 1
    case ANCILLARY_PRESET = 2

    public var code: Int { rawValue }
}

public enum LogLevels: Int, CaseIterable, Sendable {
    case LEVEL_TRACE = 0
    case LEVEL_DEBUG = 1
    case LEVEL_INFO = 2
    case LEVEL_WARN = 3
    case LEVEL_ERROR = 4
    case LEVEL_CRITICAL = 5
    case LEVEL_OFF = 6

    public var code: Int { rawValue }
}

public enum NoiseTypes: Int, CaseIterable, Sendable {
    case FIFTY = 0
    case SIXTY = 1
    case FIFTY_AND_SIXTY = 2

    public var code: Int { rawValue }
}

public enum WaveletDenoisingTypes: Int, CaseIterable, Sendable {
    case VISUSHRINK = 0
    case SURESHRINK = 1

    public var code: Int { rawValue }
}

public enum ThresholdTypes: Int, CaseIterable, Sendable {
    case SOFT = 0
    case HARD = 1

    public var code: Int { rawValue }
}

public enum WaveletExtensionTypes: Int, CaseIterable, Sendable {
    case SYMMETRIC = 0
    case PERIODIC = 1

    public var code: Int { rawValue }
}

public enum NoiseEstimationLevelTypes: Int, CaseIterable, Sendable {
    case FIRST_LEVEL = 0
    case ALL_LEVELS = 1

    public var code: Int { rawValue }
}

public enum WaveletTypes: Int, CaseIterable, Sendable {
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

    public var code: Int { rawValue }
}

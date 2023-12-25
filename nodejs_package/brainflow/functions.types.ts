import {
    AggOperations,
    BoardIds,
    BrainFlowExitCodes,
    BrainFlowPresets,
    FilterTypes,
    LogLevels,
    NoiseEstimationLevelTypes,
    NoiseTypes,
    ThresholdTypes,
    WaveletDenoisingTypes,
    WaveletExtensionTypes,
    WaveletTypes,
    WindowOperations
} from './brainflow.types';

export enum BoardControllerCLikeFunctions {
    // logging and version methods
    set_log_level_board_controller = 'int set_log_level_board_controller (int log_level)',
    set_log_file_board_controller = 'int set_log_file_board_controller (const char *log_file)',
    log_message_board_controller =
        'int log_message_board_controller (int log_level, char *log_message)',
    get_version_board_controller =
        'int get_version_board_controller (_Inout_ char *version, _Inout_ int *len, int max)',
    // board methods
    start_stream =
        'int start_stream (int buffer_size, const char *streamer_params, int board_id, const char *json_brainflow_input_params)',
    stop_stream = 'int stop_stream (int board_id, const char *json_brainflow_input_params)',
    prepare_session = 'int prepare_session (int board_id, const char *json_brainflow_input_params)',
    release_all_sessions = 'int release_all_sessions ()',
    release_session = 'int release_session (int board_id, const char *json_brainflow_input_params)',
    add_streamer =
        'int add_streamer (const char *streamer, int preset, int board_id, const char *json_brainflow_input_params)',
    config_board =
        'int config_board (const char *config, _Inout_ char *response, _Inout_ int *resp_len, int board_id, const char *json_brainflow_input_params)',
    config_board_with_bytes =
        'int config_board_with_bytes (const char *bytes, int len, int board_id, const char *json_brainflow_input_params)',
    delete_streamer =
        'int delete_streamer (const char *streamer, int preset, int board_id, const char *json_brainflow_input_params)',
    insert_marker =
        'int insert_marker (double value, int preset, int board_id, const char *json_brainflow_input_params)',
    is_prepared =
        'int is_prepared (_Inout_ int *prepared, int board_id, const char *json_brainflow_input_params)',
    get_board_data_count =
        'int get_board_data_count (int preset, _Inout_ int *result, int board_id, const char *json_brainflow_input_params)',
    get_board_data =
        'int get_board_data (int data_count, int preset, _Inout_ double *data_buf, int board_id, const char *json_brainflow_input_params)',
    get_current_board_data =
        'int get_current_board_data (int num_samples, int preset, _Inout_ double *data_buf, _Inout_ int *returned_samples, int board_id, const char *json_brainflow_input_params)',
    // board info getter methods
    get_num_rows = 'int get_num_rows (int board_id, int preset, _Inout_ int *num_rows)',
    get_sampling_rate =
        'int get_sampling_rate (int board_id, int preset, _Inout_ int *sampling_rate)',
    get_battery_channel = 'int get_battery_channel (int board_id, int preset, _Inout_ int *value)',
    get_package_num_channel =
        'int get_package_num_channel (int board_id, int preset, _Inout_ int *value)',
    get_timestamp_channel =
        'int get_timestamp_channel (int board_id, int preset, _Inout_ int *value)',
    get_marker_channel = 'int get_marker_channel (int board_id, int preset, _Inout_ int *value)',
    get_eeg_channels =
        'int get_eeg_channels (int board_id, int preset, _Inout_ int *eeg_channels, _Inout_ int *len)',
    get_exg_channels =
        'int get_exg_channels (int board_id, int preset, _Inout_ int *channels, _Inout_ int *len)',
    get_emg_channels =
        'int get_emg_channels (int board_id, int preset, _Inout_ int *channels, _Inout_ int *len)',
    get_ecg_channels =
        'int get_ecg_channels (int board_id, int preset, _Inout_ int *channels, _Inout_ int *len)',
    get_eog_channels =
        'int get_eog_channels (int board_id, int preset, _Inout_ int *channels, _Inout_ int *len)',
    get_ppg_channels =
        'int get_ppg_channels (int board_id, int preset, _Inout_ int *channels, _Inout_ int *len)',
    get_eda_channels =
        'int get_eda_channels (int board_id, int preset, _Inout_ int *channels, _Inout_ int *len)',
    get_accel_channels =
        'int get_accel_channels (int board_id, int preset, _Inout_ int *channels, _Inout_ int *len)',
    get_analog_channels =
        'int get_analog_channels (int board_id, int preset, _Inout_ int *channels, _Inout_ int *len)',
    get_gyro_channels =
        'int get_gyro_channels (int board_id, int preset, _Inout_ int *channels, _Inout_ int *len)',
    get_other_channels =
        'int get_other_channels (int board_id, int preset, _Inout_ int *channels, _Inout_ int *len)',
    get_temperature_channels =
        'int get_temperature_channels (int board_id, int preset, _Inout_ int *channels, _Inout_ int *len)',
    get_resistance_channels =
        'int get_resistance_channels (int board_id, int preset, _Inout_ int *channels, _Inout_ int *len)',
    get_magnetometer_channels =
        'int get_magnetometer_channels (int board_id, int preset, _Inout_ int *channels, _Inout_ int *len)',
    get_rotation_channels =
        'int get_rotation_channels (int board_id, int preset, _Inout_ int *channels, _Inout_ int *len)',
    get_eeg_names =
        'int get_eeg_names (int board_id, int preset, _Inout_ char *eeg_names, _Inout_ int *len)',
    get_device_name =
        'int get_device_name (int board_id, int preset, _Inout_ char *device_name, _Inout_ int *len)',
    get_board_descr =
        'int get_board_descr (int board_id, int preset, _Inout_ char *descr, _Inout_ int *len)',
    get_board_presets =
        'int get_board_presets (int board_id, _Inout_ int *presets, _Inout_ int *len)',
}

export class BoardControllerFunctions
{
    // logging and version methods
    setLogLevelBoardController!: (logLevel: LogLevels) => BrainFlowExitCodes;
    setLogFileBoardController!: (logFile: string) => BrainFlowExitCodes;
    logMessageBoardController!: (logLevel: LogLevels, logMessage: string) => BrainFlowExitCodes;
    getVersionBoardController!: (
        version: string[],
        len: number[],
        maxLen: number,
        ) => BrainFlowExitCodes;
    // board methods
    releaseAllSessions!: () => BrainFlowExitCodes;
    releaseSession!: (boardId: BoardIds, inputJson: string) => BrainFlowExitCodes;
    stopStream!: (boardId: BoardIds, inputJson: string) => BrainFlowExitCodes;
    addStreamer!: (
        streamer: string,
        preset: BrainFlowPresets,
        boardId: BoardIds,
        inputJson: string,
        ) => BrainFlowExitCodes;
    deleteStreamer!: (
        streamer: string,
        preset: BrainFlowPresets,
        boardId: BoardIds,
        inputJson: string,
        ) => BrainFlowExitCodes;
    insertMarker!: (value: number, preset: BrainFlowPresets, boardId: BoardIds,
        inputJson: string) => BrainFlowExitCodes;
    isPrepared!: (prepared: number[], boardId: BoardIds, inputJson: string) => BrainFlowExitCodes;
    prepareSession!: (boardId: BoardIds, inputJson: string) => BrainFlowExitCodes;
    startStream!: (
        numSamples: number,
        streamerParams: string|null,
        boardId: BoardIds,
        inputJson: string,
        ) => BrainFlowExitCodes;
    configBoard!: (
        config: string,
        response: string[],
        responseLen: number[],
        boardId: BoardIds,
        inputJson: string,
        ) => BrainFlowExitCodes;
    configBoardWithBytes!: (
        config: string,
        len: number,
        boardId: BoardIds,
        inputJson: string,
        ) => BrainFlowExitCodes;
    getBoardDataCount!: (
        preset: BrainFlowPresets,
        dataSize: number[],
        boardId: BoardIds,
        inputJson: string,
        ) => BrainFlowExitCodes;
    getBoardData!: (
        dataSize: number,
        preset: BrainFlowPresets,
        dataArr: number[],
        boardId: BoardIds,
        inputJson: string,
        ) => BrainFlowExitCodes;
    getCurrentBoardData!: (
        numSamples: number,
        preset: BrainFlowPresets,
        dataBuf: number[],
        returnedSamples: number[],
        boardId: BoardIds,
        inputJson: string,
        ) => BrainFlowExitCodes;
    // board info getter methods
    getNumRows!: (
        boardId: BoardIds, preset: BrainFlowPresets, numRows: number[]) => BrainFlowExitCodes;
    getTimestampChannel!: (
        boardId: BoardIds, preset: BrainFlowPresets, numRows: number[]) => BrainFlowExitCodes;
    getPackageNumChannel!: (
        boardId: BoardIds, preset: BrainFlowPresets, numRows: number[]) => BrainFlowExitCodes;
    getMarkerChannel!: (
        boardId: BoardIds, preset: BrainFlowPresets, numRows: number[]) => BrainFlowExitCodes;
    getBatteryChannel!: (
        boardId: BoardIds, preset: BrainFlowPresets, numRows: number[]) => BrainFlowExitCodes;
    getSamplingRate!: (
        boardId: BoardIds, preset: BrainFlowPresets, samplingRate: number[]) => BrainFlowExitCodes;
    getEegChannels!: (
        boardId: BoardIds,
        preset: BrainFlowPresets,
        eegChannels: number[],
        len: number[],
        ) => BrainFlowExitCodes;
    getExgChannels!: (
        boardId: BoardIds,
        preset: BrainFlowPresets,
        exgChannels: number[],
        len: number[],
        ) => BrainFlowExitCodes;
    getEmgChannels!: (
        boardId: BoardIds,
        preset: BrainFlowPresets,
        emgChannels: number[],
        len: number[],
        ) => BrainFlowExitCodes;
    getEogChannels!: (
        boardId: BoardIds,
        preset: BrainFlowPresets,
        eogChannels: number[],
        len: number[],
        ) => BrainFlowExitCodes;
    getEcgChannels!: (
        boardId: BoardIds,
        preset: BrainFlowPresets,
        ecgChannels: number[],
        len: number[],
        ) => BrainFlowExitCodes;
    getPpgChannels!: (
        boardId: BoardIds,
        preset: BrainFlowPresets,
        ppgChannels: number[],
        len: number[],
        ) => BrainFlowExitCodes;
    getEdaChannels!: (
        boardId: BoardIds,
        preset: BrainFlowPresets,
        edaChannels: number[],
        len: number[],
        ) => BrainFlowExitCodes;
    getAccelChannels!: (
        boardId: BoardIds,
        preset: BrainFlowPresets,
        accelChannels: number[],
        len: number[],
        ) => BrainFlowExitCodes;
    getAnalogChannels!: (
        boardId: BoardIds,
        preset: BrainFlowPresets,
        analogChannels: number[],
        len: number[],
        ) => BrainFlowExitCodes;
    getGyroChannels!: (
        boardId: BoardIds,
        preset: BrainFlowPresets,
        gyroChannels: number[],
        len: number[],
        ) => BrainFlowExitCodes;
    getOtherChannels!: (
        boardId: BoardIds,
        preset: BrainFlowPresets,
        otherChannels: number[],
        len: number[],
        ) => BrainFlowExitCodes;
    getRotationChannels!: (
        boardId: BoardIds,
        preset: BrainFlowPresets,
        channels: number[],
        len: number[],
        ) => BrainFlowExitCodes;
    getTemperatureChannels!: (
        boardId: BoardIds,
        preset: BrainFlowPresets,
        temperatureChannels: number[],
        len: number[],
        ) => BrainFlowExitCodes;
    getResistanceChannels!: (
        boardId: BoardIds,
        preset: BrainFlowPresets,
        resistanceChannels: number[],
        len: number[],
        ) => BrainFlowExitCodes;
    getMagnetometerChannels!: (
        boardId: BoardIds,
        preset: BrainFlowPresets,
        magnetometerChannels: number[],
        len: number[],
        ) => BrainFlowExitCodes;
    getEegNames!: (
        boardId: BoardIds,
        preset: BrainFlowPresets,
        names: string[],
        len: number[],
        ) => BrainFlowExitCodes;
    getDeviceName!: (
        boardId: BoardIds,
        preset: BrainFlowPresets,
        name: string[],
        len: number[],
        ) => BrainFlowExitCodes;
    getBoardPresets!: (
        boardId: BoardIds,
        presets: number[],
        len: number[],
        ) => BrainFlowExitCodes;
    getBoardDescr!: (
        boardId: BoardIds,
        preset: BrainFlowPresets,
        descr: string[],
        len: number[],
        ) => BrainFlowExitCodes;
}

export enum DataHandlerCLikeFunctions {
    // signal processing methods
    perform_lowpass =
        'int perform_lowpass (_Inout_ double *data, int data_len, int sampling_rate, double cutoff, int order, int filter_type, double ripple)',
    perform_highpass =
        'int perform_highpass (_Inout_ double *data, int data_len, int sampling_rate, double cutoff, int order, int filter_type, double ripple)',
    perform_bandpass =
        'int perform_bandpass (_Inout_ double *data, int data_len, int sampling_rate, double start_freq, double stop_freq, int order, int filter_type, double ripple)',
    perform_bandstop =
        'int perform_bandstop (_Inout_ double *data, int data_len, int sampling_rate, double start_freq, double stop_freq, int order, int filter_type, double ripple)',
    remove_environmental_noise =
        'int remove_environmental_noise (_Inout_ double *data, int data_len, int sampling_rate, int noise_type)',
    write_file =
        'int write_file (const double *data, int num_rows, int num_cols, const char *file_name, const char *file_mode)',
    read_file =
        'int read_file (_Inout_ double *data, _Inout_ int *num_rows, _Inout_ int *num_cols, const char *file_name, int num_elements)',
    get_num_elements_in_file =
        'int get_num_elements_in_file (const char *file_name, _Inout_ int *num_elements)',
    perform_downsampling =
        'int perform_downsampling (double *data, int data_len, int period, int agg_operation, _Inout_ double *output_data)',
    perform_wavelet_transform =
        'int perform_wavelet_transform (double *data, int data_len, int wavelet, int decomposition_level, int extension, _Inout_ double *output_data, _Inout_ int *decomposition_lengths)',
    perform_inverse_wavelet_transform =
        'int perform_inverse_wavelet_transform (double *wavelet_coeffs, int original_data_len, int wavelet, int decomposition_level, int extension, int *decomposition_lengths, _Inout_ double *output_data)',
    perform_wavelet_denoising =
        'int perform_wavelet_denoising (_Inout_ double *data, int data_len, int wavelet, int decomposition_level, int wavelet_denoising, int threshold, int extenstion_type, int noise_level)',
    get_window =
        'int get_window (int window_function, int window_len, _Inout_ double *output_window)',
    perform_fft =
        'int perform_fft (double *data, int data_len, int window_function, _Inout_ double *output_re, _Inout_ double *output_im)',
    perform_ifft =
        'int perform_ifft (double *input_re, double *input_im, int data_len, _Inout_ double *restored_data)',
    get_nearest_power_of_two = 'int get_nearest_power_of_two (int value, _Inout_ int *output)',
    get_psd =
        'int get_psd (double *data, int data_len, int sampling_rate, int window_function, _Inout_ double *output_ampl, _Inout_ double *output_freq)',
    detrend = 'int detrend (_Inout_ double *data, int data_len, int detrend_operation)',
    calc_stddev =
        'int calc_stddev (double *data, int start_pos, int end_pos, _Inout_ double *output)',
    get_psd_welch =
        'int get_psd_welch (double *data, int data_len, int nfft, int overlap, int sampling_rate, int window_function, _Inout_ double *output_ampl, _Inout_ double *output_freq)',
    get_band_power =
        'int get_band_power (double *ampl, double *freq, int data_len, double freq_start, double freq_end, _Inout_ double *band_power)',
    get_custom_band_powers =
        'int get_custom_band_powers (double *raw_data, int rows, int cols, double *start_freqs, double *stop_freqs, int num_bands, int sampling_rate, int apply_filters, _Inout_ double *avg_band_powers, _Inout_ double *stddev_band_powers)',
    get_oxygen_level =
        'int get_oxygen_level (double *ppg_ir, double *ppg_red, int data_size, int sampling_rate, double callib_coef1, double callib_coef2, double callib_coef3, _Inout_ double *oxygen_level)',
    get_heart_rate =
        'int get_heart_rate (double *ppg_ir, double *ppg_red, int data_size, int sampling_rate, int fft_size, _Inout_ double *rate)',
    restore_data_from_wavelet_detailed_coeffs =
        'int restore_data_from_wavelet_detailed_coeffs (double *data, int data_len, int wavelet, int decomposition_level, int level_to_restore, _Inout_ double *output)',
    detect_peaks_z_score =
        'int detect_peaks_z_score (double *data, int data_len, int lag, double threshold, double influence, _Inout_ double *output)',
    perform_ica =
        'int perform_ica (double *data, int rows, int cols, int num_components, _Inout_ double *w_mat, _Inout_ double *k_mat, _Inout_ double *a_mat, _Inout_ double *s_mat)',
    get_csp =
        'int get_csp (const double *data, const double *labels, int n_epochs, int n_channels, int n_times, _Inout_ double *output_w, _Inout_ double *output_d)',
    get_railed_percentage =
        'int get_railed_percentage (double *raw_data, int data_len, int gain, _Inout_ double *output)',
    // logging methods
    set_log_level_data_handler = 'int set_log_level_data_handler (int log_level)',
    set_log_file_data_handler = 'int set_log_file_data_handler (const char *log_file)',
    log_message_data_handler = 'int log_message_data_handler (int log_level, char *message)',
    get_version_data_handler =
        'int get_version_data_handler (_Inout_ char *version, _Inout_ int *num_chars, int max_chars)'
}

export class DataHandlerFunctions
{
    // signal processing methods
    getRailedPercentage!: (
        rawData: number[], dataLen: number, gain: number, output: number[]) => BrainFlowExitCodes;
    performLowPass!: (rawData: number[], dataLen: number, samplingRate: number, cutoff: number,
        order: number, filterType: FilterTypes, ripple: number) => BrainFlowExitCodes;
    performHighPass!: (rawData: number[], dataLen: number, samplingRate: number, cutoff: number,
        order: number, filterType: FilterTypes, ripple: number) => BrainFlowExitCodes;
    performBandPass!: (rawData: number[], dataLen: number, samplingRate: number, startFreq: number,
        stopFreq: number, order: number, filterType: FilterTypes,
        ripple: number) => BrainFlowExitCodes;
    performBandStop!: (rawData: number[], dataLen: number, samplingRate: number, startFreq: number,
        stopFreq: number, order: number, filterType: FilterTypes,
        ripple: number) => BrainFlowExitCodes;
    removeEnvironmentalNoise!: (rawData: number[], dataLen: number, samplingRate: number,
        noiseType: NoiseTypes) => BrainFlowExitCodes;
    writeFile!: (data: number[], rows: number, cols: number, file: string,
        mode: string) => BrainFlowExitCodes;
    readFile!: (data: number[], rows: number[], cols: number[], file: string,
        numElems: number) => BrainFlowExitCodes;
    getNumElementsInFile!: (file: string, numElems: number[]) => BrainFlowExitCodes;
    performDownsampling!: (data: number[], dataLen: number, period: number,
        aggOperation: AggOperations, output: number[]) => BrainFlowExitCodes;
    performWaveletTransform!: (data: number[], dataLen: number, wavelet: WaveletTypes,
        decompositionLevel: number, extension: WaveletExtensionTypes, outputData: number[],
        decompositionLengths: number[]) => BrainFlowExitCodes;
    performInverseWaveletTransform!: (waveletCoeffs: number[], originalDataLen: number,
        wavelet: WaveletTypes, decompositionLevel: number, extension: WaveletExtensionTypes,
        decompositionLengths: number[], outputData: number[]) => BrainFlowExitCodes;
    performWaveletDenoising!: (data: number[], dataLen: number, wavelet: WaveletTypes,
        decompositionLevel: number, waveletDenoising: WaveletDenoisingTypes,
        threshold: ThresholdTypes, extenstionType: WaveletExtensionTypes,
        noiseLevel: NoiseEstimationLevelTypes) => BrainFlowExitCodes;
    getWindow!: (windowFunction: WindowOperations, windowLen: number,
        outputWindow: number[]) => BrainFlowExitCodes;
    performFft!: (data: number[], dataLen: number, windowFunction: WindowOperations,
        outputRe: number[], outputIm: number[]) => BrainFlowExitCodes;
    performIfft!: (inputRe: number[], inputIm: number[], dataLen: number,
        restoredData: number[]) => BrainFlowExitCodes;
    getNearestPowerOfTwo!: (value: number, output: number[]) => BrainFlowExitCodes;
    getPsd!: (data: number[], dataLen: number, samplingRate: number,
        windowFunction: WindowOperations, outputAmpl: number[],
        outputFreq: number[]) => BrainFlowExitCodes;
    getPsdWelch!: (data: number[], dataLen: number, nfft: number, overlap: number,
        samplingRate: number, windowFunction: WindowOperations, outputAmpl: number[],
        outputFreq: number[]) => BrainFlowExitCodes;
    getBandPower!: (ampl: number[], freq: number[], dataLen: number, freqStart: number,
        freqEnd: number, bandPower: number[]) => BrainFlowExitCodes;
    getCustomBandPowers!: (data: number[], rows: number, cols: number, startFreqs: number[],
        stopFreqs: number[], numBands: number, samplingRate: number, applyFilters: number,
        avgBandPowers: number[], stddevBandPowers: number[]) => BrainFlowExitCodes;
    getOxygenLevel!: (ppgIr: number[], ppgRed: number[], dataSize: number, samplingRate: number,
        callibCoef1: number, callibCoef2: number, callibCoef3: number,
        oxygenLevel: number[]) => BrainFlowExitCodes;
    getHeartRate!: (ppgIr: number[], ppgRed: number[], dataSize: number, samplingRate: number,
        fftSize: number, rate: number[]) => BrainFlowExitCodes;
    restoreDataFromWaveletDetailedCoeffs!: (data: number[], dataLen: number, wavelet: number,
        decompositionLevel: number, levelToRestore: number, output: number[]) => BrainFlowExitCodes;
    detectPeaksZScore!: (data: number[], dataLen: number, lag: number, threshold: number,
        influence: number, output: number[]) => BrainFlowExitCodes;
    performIca!: (data: number[], rows: number, cols: number, numComponents: number, wMat: number[],
        kMat: number[], aMat: number[], sMat: number[]) => BrainFlowExitCodes;
    getCsp!: (data: number[], labels: number[], nEpochs: number, nChannels: number, nTimes: number,
        outputW: number[], outputD: number[]) => BrainFlowExitCodes;
    detrend!: (rawData: number[], dataLen: number, detrendOperation: number) => BrainFlowExitCodes;
    calcStddev!: (rawData: number[], startPos: number, stopPos: number,
        output: number[]) => BrainFlowExitCodes;
    // logging methods
    setLogLevelDataHandler!: (logLevel: LogLevels) => BrainFlowExitCodes;
    setLogFileDataHandler!: (logFile: string) => BrainFlowExitCodes;
    logMessageDataHandler!: (logLevel: LogLevels, logMessage: string) => BrainFlowExitCodes;
    getVersionDataHandler!: (
        version: string[],
        len: number[],
        maxLen: number,
        ) => BrainFlowExitCodes;
}

export enum MLModuleCLikeFunctions {
    // logging and version methods
    set_log_level_ml_module = 'int set_log_level_ml_module (int log_level)',
    set_log_file_ml_module = 'int set_log_file_ml_module (const char *log_file)',
    log_message_ml_module = 'int log_message_ml_module (int log_level, char *log_message)',
    get_version_ml_module =
        'int get_version_ml_module (_Inout_ char *version, _Inout_ int *len, int max)',
    prepare = 'int prepare (const char *json_params)',
    predict =
        'int predict (double *data, int data_len, _Inout_ double *output, _Inout_ int *output_len, const char *json_params)',
    release = 'int release (const char *json_params)',
    release_all = 'int release_all ()'
}

export class MLModuleFunctions
{
    // logging and version methods
    setLogLevelMLModule!: (logLevel: LogLevels) => BrainFlowExitCodes;
    setLogFileMLModule!: (logFile: string) => BrainFlowExitCodes;
    logMessageMLModule!: (logLevel: LogLevels, logMessage: string) => BrainFlowExitCodes;
    getVersionMLModule!: (
        version: string[],
        len: number[],
        maxLen: number,
        ) => BrainFlowExitCodes;
    prepare!: (inputJson: string) => BrainFlowExitCodes;
    predict!: (data: number[], dataLen: number, output: number[], outputLen: number[],
        inputJson: string) => BrainFlowExitCodes;
    releaseAll!: () => BrainFlowExitCodes;
    release!: (inputJson: string) => BrainFlowExitCodes;
}
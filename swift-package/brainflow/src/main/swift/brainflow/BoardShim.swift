//
//  BoardShim.swift
//  a Swift binding for BrainFlow's board_shim high-level API
//
//  Created by Scott Miller for Aeris Rising, LLC on 8/23/21.
//

import Foundation

/**
 * enable BrainFlow logger with level INFO
 */
func enableBoardLogger () {
    set_log_level (LogLevels.LEVEL_INFO.rawValue)
}

/**
 * enable BrainFlow logger with level TRACE
 */
func enableDevBoardLogger () {
    set_log_level (LogLevels.LEVEL_TRACE.rawValue)
}

/**
* disable BrainFlow logger
*/
func disableBoardLogger () {
    set_log_level (LogLevels.LEVEL_OFF.rawValue)
}

/**
 * redirect logger from stderr to a file
 */
func setLogFile (_ logFile: String) throws
{
    var cLogFile = logFile.cString(using: String.Encoding.utf8)!
    let result = set_log_file(&cLogFile)
    try checkErrorCode(errorMsg: "failed to set log file", errorCode: result)
}

/**
 * set log level
 */
func setLogLevel (_ logLevel: LogLevels) throws
{
    let result = set_log_level (logLevel.rawValue)
    try checkErrorCode(errorMsg: "failed to set log level", errorCode: result)
}

/**
 * send user defined strings to BrainFlow logger
 */
func logMessage (logLevel: Int32, message: String) throws {
    var cMessage = message.cString(using: String.Encoding.utf8)!
    let result = log_message (logLevel, &cMessage)
    try checkErrorCode(errorMsg: "Error in log_message", errorCode: result)
}

/**
 * get sampling rate for this board
 */
func getSamplingRate (boardId: BoardIds) throws -> Int32 {
    var samplingRate: Int32 = 0
    let result = get_sampling_rate (boardId.rawValue, &samplingRate)
    try checkErrorCode(errorMsg: "Error in board info getter", errorCode: result)

    return samplingRate
}

/**
 * get row index in returned by get_board_data() 2d array which contains timestamps
 */
func getTimestampChannel (boardId: BoardIds) throws -> Int32 {
    var timestampChannel: Int32 = 0
    let result = get_timestamp_channel (boardId.rawValue, &timestampChannel)
    try checkErrorCode(errorMsg: "Error in board info getter", errorCode: result)

    return timestampChannel
}

/**
 * get row index in returned by get_board_data() 2d array which contains markers
 */
func getMarkerChannel (boardId: BoardIds) throws -> Int32
{
    var markerChannel: Int32 = 0
    let result = get_marker_channel (boardId.rawValue, &markerChannel)
    try checkErrorCode(errorMsg: "Error in board info getter", errorCode: result)

    return markerChannel
}

/**
 * get number of rows in returned by get_board_data() 2d array
 */
func getNumRows (boardId: BoardIds) throws -> Int32 {
    var numRows: Int32 = 0
    let result = get_num_rows (boardId.rawValue, &numRows)
    try checkErrorCode(errorMsg: "Error in board info getter", errorCode: result)

    return numRows
}

/**
 * get row index in returned by get_board_data() 2d array which contains package nums
 */
func getPackageNumChannel (boardId: BoardIds) throws -> Int32 {
    var pkgNumChannel: Int32 = 0
    let result = get_package_num_channel (boardId.rawValue, &pkgNumChannel)
    try checkErrorCode(errorMsg: "Error in board info getter", errorCode: result)

    return pkgNumChannel
}

/**
 * get row index in returned by get_board_data() 2d array which contains battery level
 */
func getBatteryChannel (boardId: BoardIds) throws -> Int32 {
    var batteryChannel: Int32 = 0
    let result = get_battery_channel (boardId.rawValue, &batteryChannel)
    try checkErrorCode(errorMsg: "Error in board info getter", errorCode: result)

    return batteryChannel
}

/**
 * Get names of EEG electrodes in 10-20 system. Only if electrodes have freezed
 * locations
 */
func getEEGnames (boardId: BoardIds) throws -> [String] {
    var stringLen: Int32 = 0
    var bytes = [CChar](repeating: 0, count: 4096)
    let result = get_eeg_names (boardId.rawValue, &bytes, &stringLen)
    try checkErrorCode(errorMsg: "Error in board info getter", errorCode: result)
    let EEGnames = bytes.toString(stringLen)

    return EEGnames.components(separatedBy: ",")
}

/**
 * Get board description
 */
func getBoardDescr (boardId: BoardIds) throws -> String {
    var boardDescrStr = [CChar](repeating: CChar(0), count: 16000)
    var stringLen: Int32 = 0
    let result = get_board_descr (boardId.rawValue, &boardDescrStr, &stringLen)
    try checkErrorCode(errorMsg: "failed to get board info", errorCode: result)

    if let description = String(data: Data(bytes: &boardDescrStr, count: Int(stringLen)), encoding: .utf8) {
        return description }
    else {
        return "no data found"
    }
}

/**
 * Get device name
 */
func getDeviceName (boardId: BoardIds) throws -> String {
    var stringLen: Int32 = 0
    var deviceName = [CChar](repeating: CChar(0), count: 4096)
    let result = get_device_name (boardId.rawValue, &deviceName, &stringLen)
    try checkErrorCode(errorMsg: "Error in board info getter", errorCode: result)

    return deviceName.toString(stringLen)
}

/**
 * get row indices in returned by get_board_data() 2d array which contain EEG
 * data, for some boards we can not split EEG\EMG\... and return the same array
 */
func getEEGchannels (boardId: BoardIds) throws -> [Int32] {
    var len: Int32 = 0
    var channels = [Int32](repeating: 0, count: 512)
    let result = get_eeg_channels (boardId.rawValue, &channels, &len)
    try checkErrorCode(errorMsg: "Error in board info getter", errorCode: result)

    return Array(channels[0..<Int(len)])
}

/**
 * get row indices in returned by get_board_data() 2d array which contain EMG
 * data, for some boards we can not split EEG\EMG\... and return the same array
 */
func getEMGchannels (boardId: BoardIds) throws -> [Int32] {
    var len: Int32 = 0
    var channels = [Int32](repeating: 0, count: 512)
    let result = get_emg_channels (boardId.rawValue, &channels, &len)
    try checkErrorCode(errorMsg: "Error in board info getter", errorCode: result)

    return Array(channels[0..<Int(len)])
}

/**
 * get row indices in returned by get_board_data() 2d array which contain ECG
 * data, for some boards we can not split EEG\EMG\... and return the same array
 */
func getECGchannels (boardId: BoardIds) throws -> [Int32] {
    var len: Int32 = 0
    var channels = [Int32](repeating: 0, count: 512)
    let result = get_ecg_channels (boardId.rawValue, &channels, &len)
    try checkErrorCode(errorMsg: "Error in board info getter", errorCode: result)

    return Array(channels[0..<Int(len)])
}

/**
 * get row indices in returned by get_board_data() 2d array which contain
 * temperature data
 */
func getTemperatureChannels (boardId: BoardIds) throws -> [Int32] {
    var len: Int32 = 0
    var channels = [Int32](repeating: 0, count: 512)
    let result = get_temperature_channels (boardId.rawValue, &channels, &len)
    try checkErrorCode(errorMsg: "Error in board info getter", errorCode: result)

    return Array(channels[0..<Int(len)])
}

/**
 * get row indices in returned by get_board_data() 2d array which contain
 * resistance data
 */
func getResistanceChannels (boardId: BoardIds) throws -> [Int32] {
    var len: Int32 = 0
    var channels = [Int32](repeating: 0, count: 512)
    let result = get_resistance_channels (boardId.rawValue, &channels, &len)
    try checkErrorCode(errorMsg: "Error in board info getter", errorCode: result)

    return Array(channels[0..<Int(len)])
}

/**
 * get row indices in returned by get_board_data() 2d array which contain EOG
 * data, for some boards we can not split EEG\EMG\... and return the same array
 */
func getEOGchannels (boardId: BoardIds) throws -> [Int32] {
    var len: Int32 = 0
    var channels = [Int32](repeating: 0, count: 512)
    let result = get_eog_channels (boardId.rawValue, &channels, &len)
    try checkErrorCode(errorMsg: "Error in board info getter", errorCode: result)

    return Array(channels[0..<Int(len)])
}

/**
 * get row indices in returned by get_board_data() 2d array which contain EXG
 * data
 */
func getEXGchannels (boardId: BoardIds) throws -> [Int32] {
    var len: Int32 = 0
    var channels = [Int32](repeating: 0, count: 512)
    let result = get_exg_channels (boardId.rawValue, &channels, &len)
    try checkErrorCode(errorMsg: "Error in board info getter", errorCode: result)

    return Array(channels[0..<Int(len)])
}

/**
 * get row indices in returned by get_board_data() 2d array which contain EDA
 * data, for some boards we can not split EEG\EMG\... and return the same array
 */
func getEDAchannels (boardId: BoardIds) throws -> [Int32] {
    var len: Int32 = 0
    var channels = [Int32](repeating: 0, count: 512)
    let result = get_eda_channels (boardId.rawValue, &channels, &len)
    try checkErrorCode(errorMsg: "Error in board info getter", errorCode: result)

    return Array(channels[0..<Int(len)])
}

/**
 * get row indices in returned by get_board_data() 2d array which contain PPG
 * data, for some boards we can not split EEG\EMG\... and return the same array
 */
func getPPGchannels (boardId: BoardIds) throws -> [Int32] {
    var len: Int32 = 0
    var channels = [Int32](repeating: 0, count: 512)
    let result = get_ppg_channels (boardId.rawValue, &channels, &len)
    try checkErrorCode(errorMsg: "Error in board info getter", errorCode: result)

    return Array(channels[0..<Int(len)])
}

/**
 * get row indices in returned by get_board_data() 2d array which contain accel
 * data
 */
func getAccelChannels (boardId: BoardIds) throws -> [Int32] {
    var len: Int32 = 0
    var channels = [Int32](repeating: 0, count: 512)
    let result = get_accel_channels (boardId.rawValue, &channels, &len)
    try checkErrorCode(errorMsg: "Error in board info getter", errorCode: result)

    return Array(channels[0..<Int(len)])
}

/**
 * get row indices in returned by get_board_data() 2d array which contain analog
 * data
 */
func getAnalogChannels (boardId: BoardIds) throws -> [Int32] {
    var len: Int32 = 0
    var channels = [Int32](repeating: 0, count: 512)
    let result = get_analog_channels (boardId.rawValue, &channels, &len)
    try checkErrorCode(errorMsg: "Error in board info getter", errorCode: result)

    return Array(channels[0..<Int(len)])
}

/**
  * get row indices in returned by get_board_data() 2d array which contain gyro
  * data
  */
func getGyroChannels (boardId: BoardIds) throws -> [Int32] {
    var len: Int32 = 0
    var channels = [Int32](repeating: 0, count: 512)
    let result = get_gyro_channels (boardId.rawValue, &channels, &len)
    try checkErrorCode(errorMsg: "Error in board info getter", errorCode: result)

    return Array(channels[0..<Int(len)])
}

/**
 * get row indices in returned by get_board_data() 2d array which contain other
 * data
 */
func getOtherChannels (boardId: BoardIds) throws -> [Int32] {
    var len: Int32 = 0
    var channels = [Int32](repeating: 0, count: 512)
    let result = get_other_channels (boardId.rawValue, &channels, &len)
    try checkErrorCode(errorMsg: "Error in board info getter", errorCode: result)

    return Array(channels[0..<Int(len)])
}

/*
 If the board is streaming or playback, then get the master board ID from params.other_info.
 Otherwise return the board ID itself.
 */
func getMasterBoardID(boardId: BoardIds, params: BrainFlowInputParams) throws -> BoardIds {
    guard ((boardId == BoardIds.STREAMING_BOARD) || (boardId == BoardIds.PLAYBACK_FILE_BOARD)) else {
        return boardId
    }
    if let otherInfoInt = Int32(params.other_info) {
        if let masterBoardId = BoardIds(rawValue: otherInfoInt) {
            return masterBoardId
        }
    }
    throw BrainFlowException ("need to set params.otherInfo to master board id",
                              BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR)
}

struct BoardShim {
    let boardId: BoardIds
    let masterBoardId: BoardIds
    let bfParams: BrainFlowInputParams
    private let jsonBrainFlowInputParams: [CChar]
    
    init (_ boardId: BoardIds, _ params: BrainFlowInputParams) throws {
        self.boardId = boardId
        self.bfParams = params
        self.masterBoardId = try getMasterBoardID(boardId: boardId, params: params)
        self.jsonBrainFlowInputParams = params.json().cString(using: String.Encoding.utf8)!
    }
    
    /**
     * prepare steaming session, allocate resources
     */
    func prepareSession() throws {
        var jsonBFParams = self.jsonBrainFlowInputParams
        let result = prepare_session(boardId.rawValue, &jsonBFParams)
        try checkErrorCode(errorMsg: "failed to prepare session", errorCode: result)
    }
    
    /**
     * Get Board Id, can be different than provided (playback or streaming board)
     */
    func getBoardId () throws -> BoardIds {
        return self.masterBoardId
    }
        
    /**
     * send string to a board, use this method carefully and only if you understand what you are doing
     */
    func configBoard (_ config: String) throws -> String {
        var responseLen: Int32 = 0
        var response = [CChar](repeating: CChar(0), count: 4096)
        var cConfig = config.cString(using: String.Encoding.utf8)!
        var jsonBFParams = self.jsonBrainFlowInputParams
        let result = config_board (&cConfig, &response, &responseLen, boardId.rawValue, &jsonBFParams)
        try checkErrorCode(errorMsg: "Error in config_board", errorCode: result)

        return response.toString(responseLen)
    }

    /**
     * start streaming thread, store data in internal ringbuffer and stream them
     * from brainflow at the same time
     *
     * @param buffer_size     size of internal ringbuffer
     *
     * @param streamer_params supported vals: "file://%file_name%:w",
     *                        "file://%file_name%:a",
     *                        "streaming_board://%multicast_group_ip%:%port%". Range
     *                        for multicast addresses is from "224.0.0.0" to
     *                        "239.255.255.255"
     */
    func startStream (bufferSize: Int32, streamerParams: String) throws {
        var cStreamerParams = streamerParams.cString(using: String.Encoding.utf8)!
        var jsonBFParams = self.jsonBrainFlowInputParams
        let result = start_stream (bufferSize, &cStreamerParams, boardId.rawValue, &jsonBFParams)
        try checkErrorCode(errorMsg: "failed to start stream", errorCode: result)
    }
   
    /**
     * start streaming thread, store data in internal ringbuffer
     */
    func startStream () throws {
        try startStream (bufferSize: 450000, streamerParams: "")
    }

    /**
     * start streaming thread, store data in internal ringbuffer
     */
    func startStream (bufferSize: Int32) throws {
        try startStream (bufferSize: bufferSize, streamerParams: "")
    }
    
    /**
     * stop streaming thread
     */
    func stopStream () throws {
        var jsonBFParams = self.jsonBrainFlowInputParams
        let result = stop_stream (boardId.rawValue, &jsonBFParams)
        try checkErrorCode(errorMsg: "Error in stop_stream", errorCode: result)
    }
    
    /**
     * release all resources
     */
    func releaseSession () throws {
        var jsonBFParams = self.jsonBrainFlowInputParams
        let result = release_session (boardId.rawValue, &jsonBFParams)
        try checkErrorCode(errorMsg: "failed to release session", errorCode: result)
    }
    
    /**
     * get number of packages in ringbuffer
     */
    func getBoardDataCount () throws -> Int32 {
        var dataCount: Int32 = 0
        var jsonBFParams = self.jsonBrainFlowInputParams
        let result = get_board_data_count (&dataCount, boardId.rawValue, &jsonBFParams)
        try checkErrorCode(errorMsg: "failed to get board data count", errorCode: result)
        
        return dataCount
    }

    /**
     * insert marker to data stream
     */
    func insertMarker (value: Double) throws {
        var jsonBFParams = self.jsonBrainFlowInputParams
        let result = insert_marker (value, boardId.rawValue, &jsonBFParams)
        try checkErrorCode(errorMsg: "Error in insert_marker", errorCode: result)
    }
    
    /**
     * check session status
     */
    func isPrepared () throws -> Bool  {
        var intPrepared: Int32 = 0
        var jsonBFParams = self.jsonBrainFlowInputParams
        let result = is_prepared (&intPrepared, boardId.rawValue, &jsonBFParams)
        try checkErrorCode(errorMsg: "failed to check session", errorCode: result)
        
        return NSNumber(value: intPrepared).boolValue
    }

    /**
     * get latest collected data, can return less than "num_samples", doesnt flush
     * it from ringbuffer
     */
    func getCurrentBoardData (_ numSamples: Int32) throws -> [Double]
    {
        var numRows: Int32 = 0
        var currentSize: Int32 = 0
        var jsonBFParams = self.jsonBrainFlowInputParams

        do {
            numRows = try getNumRows (boardId: getBoardId()) }
        catch {
            throw error
        }
            
        var buffer = [Double](repeating: 0.0, count: Int(numSamples * numRows))
        let result = get_current_board_data (numSamples, &buffer, &currentSize, boardId.rawValue, &jsonBFParams)
        try checkErrorCode(errorMsg: "Error in get_current_board_data", errorCode: result)
        
        return buffer
    }

    /**
     * get all data from ringbuffer and flush it
     */
    func getBoardData () throws -> [[Double]] {
        var size: Int32 = 0
        var numRows: Int32 = 0
        var jsonBFParams = self.jsonBrainFlowInputParams

        do {
            size = try getBoardDataCount()
            guard size > 0 else {
                return [[Double]]()
            }
            numRows = try getNumRows (boardId: getBoardId()) }
        catch {
            throw error
        }
        
        var buffer = [Double](repeating: 0.0, count: Int(size * numRows))
        
        let result = get_board_data (size, &buffer, boardId.rawValue, &jsonBFParams)
        try checkErrorCode(errorMsg: "failed to get board data", errorCode: result)

        return buffer.matrix2D(rowLength: Int(size))
    }
}

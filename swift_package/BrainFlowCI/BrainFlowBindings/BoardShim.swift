//
//  BoardShim.swift
//  a Swift binding for BrainFlow's board_shim high-level API
//
//  Created by Scott Miller for Aeris Rising, LLC on 8/23/21.
//

import Foundation

struct BoardShim {
    let boardId: BoardIds
    let masterBoardId: BoardIds
    let bfParams: BrainFlowInputParams
    private let jsonBrainFlowInputParams: [CChar]
    
    /*
    try:
        self.input_json = input_params.to_json().encode()
    except BaseException:
        self.input_json = input_params.to_json()
    self.board_id = board_id
    # we need it for streaming board
    if board_id == BoardIds.STREAMING_BOARD.value or board_id == BoardIds.PLAYBACK_FILE_BOARD.value:
        if input_params.master_board != BoardIds.NO_BOARD:
            self._master_board_id = input_params.master_board
        else:
            raise BrainFlowError('you need set master board id in BrainFlowInputParams',
                                 BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)
    else:
        self._master_board_id = self.board_id
    */
        
    init (_ boardId: BoardIds, _ params: BrainFlowInputParams) throws {
        self.boardId = boardId
        self.bfParams = params
        self.masterBoardId = try BoardShim.getMasterBoardID(boardId: boardId, params: params)
        self.jsonBrainFlowInputParams = params.json().cString(using: String.Encoding.utf8)!
    }
    
    /**
     * enable BrainFlow logger with level INFO
     */
    static func enableBoardLogger() throws {
        try setLogLevel(.LEVEL_INFO)
    }

    /**
     * enable BrainFlow logger with level TRACE
     */
    static func enableDevBoardLogger() throws {
        try setLogLevel(.LEVEL_TRACE)
    }

    /**
    * disable BrainFlow logger
    */
    static func disableBoardLogger() throws {
        try setLogLevel(.LEVEL_OFF)
    }

    /**
     * redirect logger from stderr to a file
     */
    static func setLogFile (_ logFile: String) throws
    {
        var cLogFile = logFile.cString(using: String.Encoding.utf8)!
        let errorCode = set_log_file_board_controller(&cLogFile)
        try checkErrorCode("failed to set log file", errorCode)
    }

    /**
     * set log level
     */
    static func setLogLevel (_ logLevel: LogLevels) throws
    {
        let errorCode = set_log_level_board_controller (logLevel.rawValue)
        try checkErrorCode("failed to set log level", errorCode)
    }

    /**
     * send user defined strings to BrainFlow logger
     */
    static func logMessage (_ logLevel: LogLevels, _ message: String) throws {
        var cMessage = message.cString(using: String.Encoding.utf8)!
        let errorCode = log_message_board_controller (logLevel.rawValue, &cMessage)
        try checkErrorCode("Error in log_message", errorCode)
    }

    /**
     * get sampling rate for this board
     */
    static func getSamplingRate (_ boardId: BoardIds, _ preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> Int32 {
        var samplingRate: Int32 = 0
        let errorCode = get_sampling_rate (boardId.rawValue, preset.rawValue, &samplingRate)
        try checkErrorCode("Error in board info getter", errorCode)

        return samplingRate
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains timestamps
     */
    static func getTimestampChannel (_ boardId: BoardIds, _ preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> Int32 {
        var timestampChannel: Int32 = 0
        let errorCode = get_timestamp_channel (boardId.rawValue, preset.rawValue, &timestampChannel)
        try checkErrorCode("Error in board info getter", errorCode)

        return timestampChannel
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains markers
     */
    static func getMarkerChannel (_ boardId: BoardIds, _ preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> Int32
    {
        var markerChannel: Int32 = 0
        let errorCode = get_marker_channel (boardId.rawValue, preset.rawValue, &markerChannel)
        try checkErrorCode("Error in board info getter", errorCode)

        return markerChannel
    }

    /**
     * get number of rows in returned by get_board_data() 2d array
     */
    static func getNumRows (_ boardId: BoardIds, _ preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> Int32 {
        var numRows: Int32 = 0
        let errorCode = get_num_rows (boardId.rawValue, preset.rawValue, &numRows)
        try checkErrorCode("Error in board info getter", errorCode)

        return numRows
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains package nums
     */
    static func getPackageNumChannel (_ boardId: BoardIds, _ preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> Int32 {
        var pkgNumChannel: Int32 = 0
        let errorCode = get_package_num_channel (boardId.rawValue, preset.rawValue, &pkgNumChannel)
        try checkErrorCode("Error in board info getter", errorCode)

        return pkgNumChannel
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains battery level
     */
    static func getBatteryChannel (_ boardId: BoardIds, _ preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> Int32 {
        var batteryChannel: Int32 = 0
        let errorCode = get_battery_channel (boardId.rawValue, preset.rawValue, &batteryChannel)
        try checkErrorCode("Error in board info getter", errorCode)

        return batteryChannel
    }

    /**
     * Get names of EEG electrodes in 10-20 system. Only if electrodes have freezed
     * locations
     */
    static func eegNames (_ boardId: BoardIds, _ preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [String] {
        var stringLen: Int32 = 0
        var bytes = [CChar](repeating: 0, count: 4096)
        let errorCode = get_eeg_names (boardId.rawValue, preset.rawValue, &bytes, &stringLen)
        try checkErrorCode("Error in board info getter", errorCode)
        let eegNames = bytes.toString(stringLen)

        return eegNames.components(separatedBy: ",")
    }

    /**
     * Get board description
     */
    static func getBoardDescr (_ boardId: BoardIds, _ preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> BoardDescription {
        var boardDescrStr = [CChar](repeating: CChar(0), count: 16000)
        var stringLen: Int32 = 0
        let errorCode = get_board_descr (boardId.rawValue, preset.rawValue, &boardDescrStr, &stringLen)
        try checkErrorCode("failed to get board info", errorCode)

        return try BoardDescription.fromJSON(boardDescrStr.toString(stringLen))
    }

    /**
     * Get device name
     */
    static func getDeviceName (_ boardId: BoardIds, _ preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> String {
        var stringLen: Int32 = 0
        var deviceName = [CChar](repeating: CChar(0), count: 4096)
        let errorCode = get_device_name (boardId.rawValue, preset.rawValue, &deviceName, &stringLen)
        try checkErrorCode("Error in board info getter", errorCode)

        return deviceName.toString(stringLen)
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain channel
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    static func getEEGchannels (_ boardId: BoardIds, _ preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_eeg_channels (boardId.rawValue, preset.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EMG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    static func getEMGchannels (_ boardId: BoardIds, _ preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_emg_channels (boardId.rawValue, preset.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain ECG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    static func getECGchannels (_ boardId: BoardIds, _ preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_ecg_channels (boardId.rawValue, preset.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain
     * temperature data
     */
    static func getTemperatureChannels (_ boardId: BoardIds, _ preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_temperature_channels (boardId.rawValue, preset.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /**
     * Get supported presets for this device
     */
    static func getBoardPresets (_ boardId: BoardIds) throws -> [Int32] {
        var len: Int32 = 0
        var presets = [Int32](repeating: 0, count: 512)
        let errorCode = get_board_presets (boardId.rawValue, &presets, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(presets[0..<Int(len)])
    }
    
    /**
     * get row indices in returned by get_board_data() 2d array which contain
     * resistance data
     */
    static func getResistanceChannels (_ boardId: BoardIds, _ preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_resistance_channels (boardId.rawValue, preset.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EOG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    static func getEOGchannels (_ boardId: BoardIds, _ preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_eog_channels (boardId.rawValue, preset.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EXG
     * data
     */
    static func getEXGchannels (_ boardId: BoardIds, _ preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_exg_channels (boardId.rawValue, preset.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EDA
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    static func getEDAchannels (_ boardId: BoardIds, _ preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_eda_channels (boardId.rawValue, preset.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain PPG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    static func getPPGchannels (_ boardId: BoardIds, _ preset: BrainFlowPresets = .ANCILLARY_PRESET) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_ppg_channels (boardId.rawValue, preset.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain accel
     * data
     */
    static func getAccelChannels (_ boardId: BoardIds, _ preset: BrainFlowPresets = .AUXILIARY_PRESET) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_accel_channels (boardId.rawValue, preset.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain analog
     * data
     */
    static func getAnalogChannels (_ boardId: BoardIds, _ preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_analog_channels (boardId.rawValue, preset.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /**
      * get row indices in returned by get_board_data() 2d array which contain gyro
      * data
      */
    static func getGyroChannels (_ boardId: BoardIds, _ preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_gyro_channels (boardId.rawValue, preset.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain other
     * data
     */
    static func getOtherChannels (_ boardId: BoardIds, _ preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_other_channels (boardId.rawValue, preset.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /// If the board is streaming or playback, then return the board ID itself. Otherwise return the master board ID.
    static func getMasterBoardID(boardId: BoardIds, params: BrainFlowInputParams) throws -> BoardIds {
        guard ((boardId == BoardIds.STREAMING_BOARD) || (boardId == BoardIds.PLAYBACK_FILE_BOARD)) else {
            return boardId
        }
        guard params.master_board != BoardIds.NO_BOARD.rawValue else {
            throw BrainFlowError ("you need set master board id in BrainFlowInputParams",
                                  BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR)
        }
        guard let masterBoardID = BoardIds(rawValue: Int32(params.master_board)) else {
            throw BrainFlowError ("you need set master board id in BrainFlowInputParams",
                                  BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR)
        }
        return masterBoardID
    }

    /// Prepare the steaming session and allocate resources.
    func prepareSession() throws {
        try? BoardShim.logMessage(.LEVEL_INFO, "prepare session")
        var jsonBFParams = self.jsonBrainFlowInputParams
        let errorCode = prepare_session(boardId.rawValue, &jsonBFParams)
        try checkErrorCode("failed to prepare session", errorCode)
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
        let errorCode = config_board (&cConfig, &response, &responseLen, boardId.rawValue, &jsonBFParams)
        try checkErrorCode("Error in config_board", errorCode)

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
        try? BoardShim.logMessage(.LEVEL_INFO, "start streaming.  buffer size: \(bufferSize)")
        var cStreamerParams = streamerParams.cString(using: String.Encoding.utf8)!
        var jsonBFParams = self.jsonBrainFlowInputParams
        let errorCode = start_stream (bufferSize, &cStreamerParams, boardId.rawValue, &jsonBFParams)
        try checkErrorCode("failed to start stream", errorCode)
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
     * add streamer
     * @param streamer_params use it to pass data packages further or store them directly during
     streaming, supported values: "file://%file_name%:w", "file://%file_name%:a",
     "streaming_board://%multicast_group_ip%:%port%"". Range for multicast addresses is from
     "224.0.0.0" to "239.255.255.255"
     */
    func addStreamer (streamerParams: String, preset: BrainFlowPresets = .DEFAULT_PRESET) throws {
        try? BoardShim.logMessage(.LEVEL_INFO, "add streamer.  streamer params: \(streamerParams)")
        var cStreamerParams = streamerParams.cString(using: String.Encoding.utf8)!
        var jsonBFParams = self.jsonBrainFlowInputParams
        let errorCode = add_streamer (&cStreamerParams, preset.rawValue, boardId.rawValue, &jsonBFParams)
        try checkErrorCode("failed to start stream", errorCode)
    }
    
    /**
     * delete streamer
     * @param streamer_params use it to pass data packages further or store them directly during
     streaming, supported values: "file://%file_name%:w", "file://%file_name%:a",
     "streaming_board://%multicast_group_ip%:%port%"". Range for multicast addresses is from
     "224.0.0.0" to "239.255.255.255"
     */
    func deleteStreamer (streamerParams: String, preset: BrainFlowPresets = .DEFAULT_PRESET) throws {
        try? BoardShim.logMessage(.LEVEL_INFO, "add streamer.  streamer params: \(streamerParams)")
        var cStreamerParams = streamerParams.cString(using: String.Encoding.utf8)!
        var jsonBFParams = self.jsonBrainFlowInputParams
        let errorCode = delete_streamer (&cStreamerParams, preset.rawValue, boardId.rawValue, &jsonBFParams)
        try checkErrorCode("failed to start stream", errorCode)
    }

    /**
     * stop streaming thread
     */
    func stopStream () throws {
        try? BoardShim.logMessage(.LEVEL_INFO, "stop streaming")
        var jsonBFParams = self.jsonBrainFlowInputParams
        let errorCode = stop_stream (boardId.rawValue, &jsonBFParams)
        try checkErrorCode("Error in stop_stream", errorCode)
    }
    
    /**
     * release all resources
     */
    func releaseSession () throws {
        try? BoardShim.logMessage(.LEVEL_INFO, "release session")
        var jsonBFParams = self.jsonBrainFlowInputParams
        let errorCode = release_session (boardId.rawValue, &jsonBFParams)
        try checkErrorCode("failed to release session", errorCode)
    }
    
    /**
     * get number of packages in ringbuffer
     */
    func getBoardDataCount (_ preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> Int32 {
        var dataCount: Int32 = 0
        var jsonBFParams = self.jsonBrainFlowInputParams
        let errorCode = get_board_data_count (preset.rawValue, &dataCount, boardId.rawValue, &jsonBFParams)
        try checkErrorCode("failed to get board data count", errorCode)
        
        return dataCount
    }

    /**
     * insert marker to data stream
     */
    func insertMarker (value: Double, _ preset: BrainFlowPresets = .DEFAULT_PRESET) throws {
        var jsonBFParams = self.jsonBrainFlowInputParams
        let errorCode = insert_marker (value, preset.rawValue, boardId.rawValue, &jsonBFParams)
        try checkErrorCode("Error in insert_marker", errorCode)
    }
    
    /**
     * check session status
     */
    func isPrepared () throws -> Bool  {
        var intPrepared: Int32 = 0
        var jsonBFParams = self.jsonBrainFlowInputParams
        let errorCode = is_prepared (&intPrepared, boardId.rawValue, &jsonBFParams)
        try checkErrorCode("failed to check session", errorCode)
        
        return NSNumber(value: intPrepared).boolValue
    }

    /**
     * get latest collected data, can return less than "num_samples", doesnt flush
     * it from ringbuffer
     */
    func getCurrentBoardData (_ numSamples: Int32, _ preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [[Double]]
    {
        var numRows: Int32 = 0
        var currentSize: Int32 = 0
        var jsonBFParams = self.jsonBrainFlowInputParams

        do {
            numRows = try BoardShim.getNumRows (getBoardId()) }
        catch {
            throw error
        }
            
        var buffer = [Double](repeating: 0.0, count: Int(numSamples * numRows))
        let errorCode = get_current_board_data (numSamples, preset.rawValue, &buffer, &currentSize,
                                                boardId.rawValue, &jsonBFParams)
        try checkErrorCode("Error in get_current_board_data", errorCode)
        
        return buffer.matrix2D(rowLength: Int(numSamples))
    }

  
    func getBoardData (_ size: Int32? = nil, _ preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [[Double]] {
        var jsonBFParams = self.jsonBrainFlowInputParams
        let numRows = try BoardShim.getNumRows (getBoardId())
        var numSamples: Int32 = 0
        if let testSize = size {
            // size is not nil
            if testSize < 1 {
                throw BrainFlowError("invalid num_samples", BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR)
            }
            numSamples = testSize
        } else {
            // size is nil, so get all available data:
            numSamples = try getBoardDataCount(preset)
        }
        var buffer = [Double](repeating: 0.0, count: Int(numSamples * numRows))
        let errorCode = get_board_data (numSamples, preset.rawValue, &buffer, boardId.rawValue, &jsonBFParams)
        try checkErrorCode("failed to get board data", errorCode)

        return buffer.matrix2D(rowLength: Int(numSamples))
    }
}

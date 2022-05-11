//
//  BoardShim.swift
//  a Swift binding for BrainFlow's board_shim high-level API
//
//  Created by Scott Miller for Aeris Rising, LLC on 8/23/21.
//

import Foundation
//import BrainFlow

struct BoardShim {
    let boardId: BoardIds
    let masterBoardId: BoardIds
    let bfParams: BrainFlowInputParams
    private let jsonBrainFlowInputParams: [CChar]
    
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
    static func getSamplingRate (_ boardId: BoardIds) throws -> Int32 {
        var samplingRate: Int32 = 0
        let errorCode = get_sampling_rate (boardId.rawValue, &samplingRate)
        try checkErrorCode("Error in board info getter", errorCode)

        return samplingRate
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains timestamps
     */
    static func getTimestampChannel (_ boardId: BoardIds) throws -> Int32 {
        var timestampChannel: Int32 = 0
        let errorCode = get_timestamp_channel (boardId.rawValue, &timestampChannel)
        try checkErrorCode("Error in board info getter", errorCode)

        return timestampChannel
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains markers
     */
    static func getMarkerChannel (_ boardId: BoardIds) throws -> Int32
    {
        var markerChannel: Int32 = 0
        let errorCode = get_marker_channel (boardId.rawValue, &markerChannel)
        try checkErrorCode("Error in board info getter", errorCode)

        return markerChannel
    }

    /**
     * get number of rows in returned by get_board_data() 2d array
     */
    static func getNumRows (_ boardId: BoardIds) throws -> Int32 {
        var numRows: Int32 = 0
        let errorCode = get_num_rows (boardId.rawValue, &numRows)
        try checkErrorCode("Error in board info getter", errorCode)

        return numRows
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains package nums
     */
    static func getPackageNumChannel (_ boardId: BoardIds) throws -> Int32 {
        var pkgNumChannel: Int32 = 0
        let errorCode = get_package_num_channel (boardId.rawValue, &pkgNumChannel)
        try checkErrorCode("Error in board info getter", errorCode)

        return pkgNumChannel
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains battery level
     */
    static func getBatteryChannel (_ boardId: BoardIds) throws -> Int32 {
        var batteryChannel: Int32 = 0
        let errorCode = get_battery_channel (boardId.rawValue, &batteryChannel)
        try checkErrorCode("Error in board info getter", errorCode)

        return batteryChannel
    }

    /**
     * Get names of EEG electrodes in 10-20 system. Only if electrodes have freezed
     * locations
     */
    static func getEEGnames (_ boardId: BoardIds) throws -> [String] {
        var stringLen: Int32 = 0
        var bytes = [CChar](repeating: 0, count: 4096)
        let errorCode = get_eeg_names (boardId.rawValue, &bytes, &stringLen)
        try checkErrorCode("Error in board info getter", errorCode)
        let EEGnames = bytes.toString(stringLen)

        return EEGnames.components(separatedBy: ",")
    }

    /**
     * Get board description
     */
    static func getBoardDescr (_ boardId: BoardIds) throws -> BoardDescription {
        var boardDescrStr = [CChar](repeating: CChar(0), count: 16000)
        var stringLen: Int32 = 0
        let errorCode = get_board_descr (boardId.rawValue, &boardDescrStr, &stringLen)
        try checkErrorCode("failed to get board info", errorCode)

        return try BoardDescription(boardDescrStr.toString(stringLen))
//        let descrData = Data(bytes: &boardDescrStr, count: Int(stringLen))
//        if let description = String(data: descrData, encoding: .utf8) {
//            return description }
//        else {
//            return "no data found"
//        }
    }

    /**
     * Get device name
     */
    static func getDeviceName (_ boardId: BoardIds) throws -> String {
        var stringLen: Int32 = 0
        var deviceName = [CChar](repeating: CChar(0), count: 4096)
        let errorCode = get_device_name (boardId.rawValue, &deviceName, &stringLen)
        try checkErrorCode("Error in board info getter", errorCode)

        return deviceName.toString(stringLen)
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EEG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    static func getEEGchannels (_ boardId: BoardIds) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_eeg_channels (boardId.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EMG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    static func getEMGchannels (_ boardId: BoardIds) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_emg_channels (boardId.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain ECG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    static func getECGchannels (_ boardId: BoardIds) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_ecg_channels (boardId.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain
     * temperature data
     */
    static func getTemperatureChannels (_ boardId: BoardIds) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_temperature_channels (boardId.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain
     * resistance data
     */
    static func getResistanceChannels (_ boardId: BoardIds) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_resistance_channels (boardId.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EOG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    static func getEOGchannels (_ boardId: BoardIds) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_eog_channels (boardId.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EXG
     * data
     */
    static func getEXGchannels (_ boardId: BoardIds) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_exg_channels (boardId.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EDA
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    static func getEDAchannels (_ boardId: BoardIds) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_eda_channels (boardId.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain PPG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    static func getPPGchannels (_ boardId: BoardIds) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_ppg_channels (boardId.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain accel
     * data
     */
    static func getAccelChannels (_ boardId: BoardIds) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_accel_channels (boardId.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain analog
     * data
     */
    static func getAnalogChannels (_ boardId: BoardIds) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_analog_channels (boardId.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /**
      * get row indices in returned by get_board_data() 2d array which contain gyro
      * data
      */
    static func getGyroChannels (_ boardId: BoardIds) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_gyro_channels (boardId.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain other
     * data
     */
    static func getOtherChannels (_ boardId: BoardIds) throws -> [Int32] {
        var len: Int32 = 0
        var channels = [Int32](repeating: 0, count: 512)
        let errorCode = get_other_channels (boardId.rawValue, &channels, &len)
        try checkErrorCode("Error in board info getter", errorCode)

        return Array(channels[0..<Int(len)])
    }

    /*
     If the board is streaming or playback, then get the master board ID from params.other_info.
     Otherwise return the board ID itself.
     */
    static func getMasterBoardID(boardId: BoardIds, params: BrainFlowInputParams) throws -> BoardIds {
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

    /**
     * prepare steaming session, allocate resources
     */
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
    func getBoardDataCount () throws -> Int32 {
        var dataCount: Int32 = 0
        var jsonBFParams = self.jsonBrainFlowInputParams
        let errorCode = get_board_data_count (&dataCount, boardId.rawValue, &jsonBFParams)
        try checkErrorCode("failed to get board data count", errorCode)
        
        return dataCount
    }

    /**
     * insert marker to data stream
     */
    func insertMarker (value: Double) throws {
        var jsonBFParams = self.jsonBrainFlowInputParams
        let errorCode = insert_marker (value, boardId.rawValue, &jsonBFParams)
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
    func getCurrentBoardData (_ numSamples: Int32) throws -> [[Double]]
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
        let errorCode = get_current_board_data (numSamples, &buffer, &currentSize, boardId.rawValue, &jsonBFParams)
        try checkErrorCode("Error in get_current_board_data", errorCode)
        
        return buffer.matrix2D(rowLength: Int(numSamples))
    }

    /**
     * get all data from ringbuffer and flush it
     */
    func getBoardData () throws -> [[Double]] {
        let size = try getBoardDataCount()
        guard size > 0 else {
            return [[Double]]()
        }
        
        return try getBoardData(size)
    }
    
    func getBoardData (_ size: Int32) throws -> [[Double]] {
        var jsonBFParams = self.jsonBrainFlowInputParams
        let numRows = try BoardShim.getNumRows (getBoardId())
        var buffer = [Double](repeating: 0.0, count: Int(size * numRows))
        
        let errorCode = get_board_data (size, &buffer, boardId.rawValue, &jsonBFParams)
        try checkErrorCode("failed to get board data", errorCode)

        return buffer.matrix2D(rowLength: Int(size))
    }
}

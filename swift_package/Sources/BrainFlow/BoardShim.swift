import Foundation

public final class BoardShim {
    private let board_id: Int
    private let input_params: BrainFlowInputParams
    private let serialized_params: String

    public init(board_id: Int, input_params: BrainFlowInputParams = BrainFlowInputParams()) throws {
        self.board_id = board_id
        self.input_params = input_params
        serialized_params = try input_params.to_json()
    }

    public convenience init(board_id: BoardIds, input_params: BrainFlowInputParams = BrainFlowInputParams()) throws {
        try self.init(board_id: board_id.rawValue, input_params: input_params)
    }

    deinit {
        try? release_session()
    }

    public func prepare_session() throws {
        try serialized_params.withCString { params in
            try BoardShimNative.withBoard { native in
                try checkBrainFlowExitCode(native.prepare_session(CInt(board_id), params), "Error in prepare_session")
            }
        }
    }

    public func start_stream(buffer_size: Int = 450_000, streamer_params: String = "") throws {
        try serialized_params.withCString { params in
            try streamer_params.withCString { streamer in
                try BoardShimNative.withBoard { native in
                    try checkBrainFlowExitCode(
                        native.start_stream(CInt(buffer_size), streamer, CInt(board_id), params),
                        "Error in start_stream"
                    )
                }
            }
        }
    }

    public func stop_stream() throws {
        try serialized_params.withCString { params in
            try BoardShimNative.withBoard { native in
                try checkBrainFlowExitCode(native.stop_stream(CInt(board_id), params), "Error in stop_stream")
            }
        }
    }

    public func release_session() throws {
        try serialized_params.withCString { params in
            try BoardShimNative.withBoard { native in
                try checkBrainFlowExitCode(native.release_session(CInt(board_id), params), "Error in release_session")
            }
        }
    }

    public func add_streamer(_ streamer: String, preset: BrainFlowPresets = .DEFAULT_PRESET) throws {
        try serialized_params.withCString { params in
            try streamer.withCString { streamerPtr in
                try BoardShimNative.withBoard { native in
                    try checkBrainFlowExitCode(
                        native.add_streamer(streamerPtr, CInt(preset.rawValue), CInt(board_id), params),
                        "Error in add_streamer"
                    )
                }
            }
        }
    }

    public func delete_streamer(_ streamer: String, preset: BrainFlowPresets = .DEFAULT_PRESET) throws {
        try serialized_params.withCString { params in
            try streamer.withCString { streamerPtr in
                try BoardShimNative.withBoard { native in
                    try checkBrainFlowExitCode(
                        native.delete_streamer(streamerPtr, CInt(preset.rawValue), CInt(board_id), params),
                        "Error in delete_streamer"
                    )
                }
            }
        }
    }

    public func config_board(_ config: String) throws -> String {
        try serialized_params.withCString { params in
            try config.withCString { configPtr in
                var response = [CChar](repeating: 0, count: 16_000)
                var responseLen: CInt = 0
                try BoardShimNative.withBoard { native in
                    try checkBrainFlowExitCode(
                        native.config_board(configPtr, &response, &responseLen, CInt(response.count), CInt(board_id), params),
                        "Error in config_board"
                    )
                }
                return String(cString: response)
            }
        }
    }

    public func config_board_with_bytes(_ bytes: [UInt8]) throws {
        guard !bytes.isEmpty else { return }
        try serialized_params.withCString { params in
            try bytes.withUnsafeBufferPointer { buffer in
                let raw = UnsafeRawPointer(buffer.baseAddress!).assumingMemoryBound(to: CChar.self)
                try BoardShimNative.withBoard { native in
                    try checkBrainFlowExitCode(
                        native.config_board_with_bytes(raw, CInt(bytes.count), CInt(board_id), params),
                        "Error in config_board_with_bytes"
                    )
                }
            }
        }
    }

    public func get_current_board_data(num_samples: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [[Double]] {
        let rows = try Self.get_num_rows(board_id: board_id, preset: preset)
        var data = [Double](repeating: 0.0, count: rows * num_samples)
        var returnedSamples: CInt = 0
        try serialized_params.withCString { params in
            try data.withUnsafeMutableBufferPointer { dataPtr in
                try BoardShimNative.withBoard { native in
                    try checkBrainFlowExitCode(
                        native.get_current_board_data(CInt(num_samples), CInt(preset.rawValue), dataPtr.baseAddress, &returnedSamples, CInt(board_id), params),
                        "Error in get_current_board_data"
                    )
                }
            }
        }
        let count = Int(returnedSamples)
        return BrainFlowArray.reshape_data_to_2d(num_rows: rows, num_cols: count, linear_buffer: Array(data.prefix(rows * count)))
    }

    public func get_board_data(_ num_datapoints: Int? = nil, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [[Double]] {
        let rows = try Self.get_num_rows(board_id: board_id, preset: preset)
        let count = try num_datapoints ?? get_board_data_count(preset: preset)
        guard count >= 0 else { throw invalidArguments("num_datapoints must be non-negative") }
        var data = [Double](repeating: 0.0, count: rows * count)
        try serialized_params.withCString { params in
            try data.withUnsafeMutableBufferPointer { dataPtr in
                try BoardShimNative.withBoard { native in
                    try checkBrainFlowExitCode(
                        native.get_board_data(CInt(count), CInt(preset.rawValue), dataPtr.baseAddress, CInt(board_id), params),
                        "Error in get_board_data"
                    )
                }
            }
        }
        return BrainFlowArray.reshape_data_to_2d(num_rows: rows, num_cols: count, linear_buffer: data)
    }

    public func get_board_data_count(preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> Int {
        try serialized_params.withCString { params in
            var result: CInt = 0
            try BoardShimNative.withBoard { native in
                try checkBrainFlowExitCode(
                    native.get_board_data_count(CInt(preset.rawValue), &result, CInt(board_id), params),
                    "Error in get_board_data_count"
                )
            }
            return Int(result)
        }
    }

    public func get_board_id() -> Int {
        board_id
    }

    public func get_board_sampling_rate(preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> Int {
        try serialized_params.withCString { params in
            var rate: CInt = 0
            try BoardShimNative.withBoard { native in
                try checkBrainFlowExitCode(
                    native.get_board_sampling_rate(CInt(preset.rawValue), &rate, CInt(board_id), params),
                    "Error in get_board_sampling_rate"
                )
            }
            return Int(rate)
        }
    }

    public func insert_marker(_ value: Double, preset: BrainFlowPresets = .DEFAULT_PRESET) throws {
        try serialized_params.withCString { params in
            try BoardShimNative.withBoard { native in
                try checkBrainFlowExitCode(
                    native.insert_marker(value, CInt(preset.rawValue), CInt(board_id), params),
                    "Error in insert_marker"
                )
            }
        }
    }

    public func is_prepared() throws -> Bool {
        try serialized_params.withCString { params in
            var prepared: CInt = 0
            try BoardShimNative.withBoard { native in
                try checkBrainFlowExitCode(native.is_prepared(&prepared, CInt(board_id), params), "Error in is_prepared")
            }
            return prepared != 0
        }
    }

    public static func release_all_sessions() throws {
        try BoardShimNative.withBoard { native in
            try checkBrainFlowExitCode(native.release_all_sessions(), "Error in release_all_sessions")
        }
    }

    public static func set_log_level(_ log_level: Int) throws {
        try BoardShimNative.withBoard { native in
            try checkBrainFlowExitCode(native.set_log_level_board_controller(CInt(log_level)), "Error in set_log_level")
        }
    }

    public static func set_log_level(_ log_level: LogLevels) throws {
        try set_log_level(log_level.rawValue)
    }

    public static func enable_board_logger() throws {
        try set_log_level(LogLevels.LEVEL_INFO)
    }

    public static func enable_dev_board_logger() throws {
        try set_log_level(LogLevels.LEVEL_TRACE)
    }

    public static func disable_board_logger() throws {
        try set_log_level(LogLevels.LEVEL_OFF)
    }

    public static func set_log_file(_ log_file: String) throws {
        try log_file.withCString { path in
            try BoardShimNative.withBoard { native in
                try checkBrainFlowExitCode(native.set_log_file_board_controller(path), "Error in set_log_file")
            }
        }
    }

    public static func log_message(_ log_level: Int, message: String) throws {
        var mutableMessage = Array(message.utf8CString)
        try mutableMessage.withUnsafeMutableBufferPointer { pointer in
            try BoardShimNative.withBoard { native in
                try checkBrainFlowExitCode(native.log_message_board_controller(CInt(log_level), pointer.baseAddress), "Error in log_message")
            }
        }
    }

    public static func log_message(_ log_level: LogLevels, message: String) throws {
        try log_message(log_level.rawValue, message: message)
    }

    public static func get_sampling_rate(board_id: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> Int {
        try getIntBoardInfo(board_id: board_id, preset: preset, function: \.get_sampling_rate)
    }

    public static func get_sampling_rate(board_id: BoardIds, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> Int {
        try get_sampling_rate(board_id: board_id.rawValue, preset: preset)
    }

    public static func get_package_num_channel(board_id: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> Int {
        try getIntBoardInfo(board_id: board_id, preset: preset, function: \.get_package_num_channel)
    }

    public static func get_package_num_channel(board_id: BoardIds, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> Int {
        try get_package_num_channel(board_id: board_id.rawValue, preset: preset)
    }

    public static func get_battery_channel(board_id: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> Int {
        try getIntBoardInfo(board_id: board_id, preset: preset, function: \.get_battery_channel)
    }

    public static func get_battery_channel(board_id: BoardIds, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> Int {
        try get_battery_channel(board_id: board_id.rawValue, preset: preset)
    }

    public static func get_num_rows(board_id: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> Int {
        try getIntBoardInfo(board_id: board_id, preset: preset, function: \.get_num_rows)
    }

    public static func get_num_rows(board_id: BoardIds, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> Int {
        try get_num_rows(board_id: board_id.rawValue, preset: preset)
    }

    public static func get_timestamp_channel(board_id: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> Int {
        try getIntBoardInfo(board_id: board_id, preset: preset, function: \.get_timestamp_channel)
    }

    public static func get_timestamp_channel(board_id: BoardIds, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> Int {
        try get_timestamp_channel(board_id: board_id.rawValue, preset: preset)
    }

    public static func get_marker_channel(board_id: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> Int {
        try getIntBoardInfo(board_id: board_id, preset: preset, function: \.get_marker_channel)
    }

    public static func get_marker_channel(board_id: BoardIds, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> Int {
        try get_marker_channel(board_id: board_id.rawValue, preset: preset)
    }

    public static func get_eeg_names(board_id: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [String] {
        let names = try getStringBoardInfo(board_id: board_id, preset: preset, maxLength: 4096, function: \.get_eeg_names)
        return names.isEmpty ? [] : names.split(separator: ",").map(String.init)
    }

    public static func get_eeg_names(board_id: BoardIds, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [String] {
        try get_eeg_names(board_id: board_id.rawValue, preset: preset)
    }

    public static func get_board_presets(board_id: Int) throws -> [BrainFlowPresets] {
        var values = [CInt](repeating: 0, count: 512)
        var length: CInt = 0
        try values.withUnsafeMutableBufferPointer { pointer in
            try BoardShimNative.withBoard { native in
                try checkBrainFlowExitCode(native.get_board_presets(CInt(board_id), pointer.baseAddress, &length), "Error in get_board_presets")
            }
        }
        return values.prefix(Int(length)).compactMap { BrainFlowPresets(rawValue: Int($0)) }
    }

    public static func get_board_presets(board_id: BoardIds) throws -> [BrainFlowPresets] {
        try get_board_presets(board_id: board_id.rawValue)
    }

    public static func get_version() throws -> String {
        try getVersion(function: \.get_version_board_controller)
    }

    public static func get_board_descr(board_id: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> String {
        try getStringBoardInfo(board_id: board_id, preset: preset, maxLength: 16_000, function: \.get_board_descr)
    }

    public static func get_board_descr(board_id: BoardIds, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> String {
        try get_board_descr(board_id: board_id.rawValue, preset: preset)
    }

    public static func get_device_name(board_id: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> String {
        try getStringBoardInfo(board_id: board_id, preset: preset, maxLength: 4096, function: \.get_device_name)
    }

    public static func get_device_name(board_id: BoardIds, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> String {
        try get_device_name(board_id: board_id.rawValue, preset: preset)
    }

    public static func get_eeg_channels(board_id: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try getChannels(board_id: board_id, preset: preset, function: \.get_eeg_channels)
    }

    public static func get_eeg_channels(board_id: BoardIds, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try get_eeg_channels(board_id: board_id.rawValue, preset: preset)
    }

    public static func get_exg_channels(board_id: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try getChannels(board_id: board_id, preset: preset, function: \.get_exg_channels)
    }

    public static func get_exg_channels(board_id: BoardIds, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try get_exg_channels(board_id: board_id.rawValue, preset: preset)
    }

    public static func get_emg_channels(board_id: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try getChannels(board_id: board_id, preset: preset, function: \.get_emg_channels)
    }

    public static func get_emg_channels(board_id: BoardIds, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try get_emg_channels(board_id: board_id.rawValue, preset: preset)
    }

    public static func get_ecg_channels(board_id: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try getChannels(board_id: board_id, preset: preset, function: \.get_ecg_channels)
    }

    public static func get_ecg_channels(board_id: BoardIds, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try get_ecg_channels(board_id: board_id.rawValue, preset: preset)
    }

    public static func get_eog_channels(board_id: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try getChannels(board_id: board_id, preset: preset, function: \.get_eog_channels)
    }

    public static func get_eog_channels(board_id: BoardIds, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try get_eog_channels(board_id: board_id.rawValue, preset: preset)
    }

    public static func get_ppg_channels(board_id: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try getChannels(board_id: board_id, preset: preset, function: \.get_ppg_channels)
    }

    public static func get_ppg_channels(board_id: BoardIds, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try get_ppg_channels(board_id: board_id.rawValue, preset: preset)
    }

    public static func get_optical_channels(board_id: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try getChannels(board_id: board_id, preset: preset, function: \.get_optical_channels)
    }

    public static func get_optical_channels(board_id: BoardIds, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try get_optical_channels(board_id: board_id.rawValue, preset: preset)
    }

    public static func get_eda_channels(board_id: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try getChannels(board_id: board_id, preset: preset, function: \.get_eda_channels)
    }

    public static func get_eda_channels(board_id: BoardIds, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try get_eda_channels(board_id: board_id.rawValue, preset: preset)
    }

    public static func get_accel_channels(board_id: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try getChannels(board_id: board_id, preset: preset, function: \.get_accel_channels)
    }

    public static func get_accel_channels(board_id: BoardIds, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try get_accel_channels(board_id: board_id.rawValue, preset: preset)
    }

    public static func get_rotation_channels(board_id: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try getChannels(board_id: board_id, preset: preset, function: \.get_rotation_channels)
    }

    public static func get_rotation_channels(board_id: BoardIds, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try get_rotation_channels(board_id: board_id.rawValue, preset: preset)
    }

    public static func get_analog_channels(board_id: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try getChannels(board_id: board_id, preset: preset, function: \.get_analog_channels)
    }

    public static func get_analog_channels(board_id: BoardIds, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try get_analog_channels(board_id: board_id.rawValue, preset: preset)
    }

    public static func get_gyro_channels(board_id: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try getChannels(board_id: board_id, preset: preset, function: \.get_gyro_channels)
    }

    public static func get_gyro_channels(board_id: BoardIds, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try get_gyro_channels(board_id: board_id.rawValue, preset: preset)
    }

    public static func get_other_channels(board_id: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try getChannels(board_id: board_id, preset: preset, function: \.get_other_channels)
    }

    public static func get_other_channels(board_id: BoardIds, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try get_other_channels(board_id: board_id.rawValue, preset: preset)
    }

    public static func get_temperature_channels(board_id: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try getChannels(board_id: board_id, preset: preset, function: \.get_temperature_channels)
    }

    public static func get_temperature_channels(board_id: BoardIds, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try get_temperature_channels(board_id: board_id.rawValue, preset: preset)
    }

    public static func get_resistance_channels(board_id: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try getChannels(board_id: board_id, preset: preset, function: \.get_resistance_channels)
    }

    public static func get_resistance_channels(board_id: BoardIds, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try get_resistance_channels(board_id: board_id.rawValue, preset: preset)
    }

    public static func get_magnetometer_channels(board_id: Int, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try getChannels(board_id: board_id, preset: preset, function: \.get_magnetometer_channels)
    }

    public static func get_magnetometer_channels(board_id: BoardIds, preset: BrainFlowPresets = .DEFAULT_PRESET) throws -> [Int] {
        try get_magnetometer_channels(board_id: board_id.rawValue, preset: preset)
    }

    public static func reshape_data_to_2d(num_rows: Int, num_cols: Int, linear_buffer: [Double]) -> [[Double]] {
        BrainFlowArray.reshape_data_to_2d(num_rows: num_rows, num_cols: num_cols, linear_buffer: linear_buffer)
    }

    private static func getIntBoardInfo(
        board_id: Int,
        preset: BrainFlowPresets,
        function: KeyPath<BoardShimNative, BoardShimNative.BoardInfoIntFunction>
    ) throws -> Int {
        var result: CInt = 0
        try BoardShimNative.withBoard { native in
            try checkBrainFlowExitCode(native[keyPath: function](CInt(board_id), CInt(preset.rawValue), &result), "Error in board info getter")
        }
        return Int(result)
    }

    private static func getChannels(
        board_id: Int,
        preset: BrainFlowPresets,
        function: KeyPath<BoardShimNative, BoardShimNative.BoardInfoChannelsFunction>
    ) throws -> [Int] {
        var channels = [CInt](repeating: 0, count: 512)
        var length: CInt = 0
        try channels.withUnsafeMutableBufferPointer { pointer in
            try BoardShimNative.withBoard { native in
                try checkBrainFlowExitCode(native[keyPath: function](CInt(board_id), CInt(preset.rawValue), pointer.baseAddress, &length), "Error in board channel getter")
            }
        }
        return channels.prefix(Int(length)).map(Int.init)
    }

    private static func getStringBoardInfo(
        board_id: Int,
        preset: BrainFlowPresets,
        maxLength: Int,
        function: KeyPath<BoardShimNative, BoardShimNative.BoardInfoStringFunction>
    ) throws -> String {
        var bytes = [CChar](repeating: 0, count: maxLength)
        var length: CInt = 0
        try bytes.withUnsafeMutableBufferPointer { pointer in
            try BoardShimNative.withBoard { native in
                try checkBrainFlowExitCode(native[keyPath: function](CInt(board_id), CInt(preset.rawValue), pointer.baseAddress, &length, CInt(maxLength)), "Error in board string getter")
            }
        }
        return String(bytes: bytes.prefix(Int(length)).map { UInt8(bitPattern: $0) }, encoding: .utf8) ?? ""
    }

    private static func getVersion(function: KeyPath<BoardShimNative, BoardShimNative.VersionFunction>) throws -> String {
        let maxLength = 64
        var bytes = [CChar](repeating: 0, count: maxLength)
        var length: CInt = 0
        try bytes.withUnsafeMutableBufferPointer { pointer in
            try BoardShimNative.withBoard { native in
                try checkBrainFlowExitCode(native[keyPath: function](pointer.baseAddress, &length, CInt(maxLength)), "Error in get_version")
            }
        }
        return String(bytes: bytes.prefix(Int(length)).map { UInt8(bitPattern: $0) }, encoding: .utf8) ?? ""
    }
}

final class BoardShimNative {
    typealias BoardInfoIntFunction = @convention(c) (CInt, CInt, UnsafeMutablePointer<CInt>?) -> CInt
    typealias BoardInfoChannelsFunction = @convention(c) (CInt, CInt, UnsafeMutablePointer<CInt>?, UnsafeMutablePointer<CInt>?) -> CInt
    typealias BoardInfoStringFunction = @convention(c) (CInt, CInt, UnsafeMutablePointer<CChar>?, UnsafeMutablePointer<CInt>?, CInt) -> CInt
    typealias VersionFunction = @convention(c) (UnsafeMutablePointer<CChar>?, UnsafeMutablePointer<CInt>?, CInt) -> CInt

    let prepare_session: @convention(c) (CInt, UnsafePointer<CChar>?) -> CInt
    let start_stream: @convention(c) (CInt, UnsafePointer<CChar>?, CInt, UnsafePointer<CChar>?) -> CInt
    let stop_stream: @convention(c) (CInt, UnsafePointer<CChar>?) -> CInt
    let release_session: @convention(c) (CInt, UnsafePointer<CChar>?) -> CInt
    let get_current_board_data: @convention(c) (CInt, CInt, UnsafeMutablePointer<Double>?, UnsafeMutablePointer<CInt>?, CInt, UnsafePointer<CChar>?) -> CInt
    let get_board_data_count: @convention(c) (CInt, UnsafeMutablePointer<CInt>?, CInt, UnsafePointer<CChar>?) -> CInt
    let get_board_data: @convention(c) (CInt, CInt, UnsafeMutablePointer<Double>?, CInt, UnsafePointer<CChar>?) -> CInt
    let get_board_sampling_rate: @convention(c) (CInt, UnsafeMutablePointer<CInt>?, CInt, UnsafePointer<CChar>?) -> CInt
    let config_board: @convention(c) (UnsafePointer<CChar>?, UnsafeMutablePointer<CChar>?, UnsafeMutablePointer<CInt>?, CInt, CInt, UnsafePointer<CChar>?) -> CInt
    let config_board_with_bytes: @convention(c) (UnsafePointer<CChar>?, CInt, CInt, UnsafePointer<CChar>?) -> CInt
    let is_prepared: @convention(c) (UnsafeMutablePointer<CInt>?, CInt, UnsafePointer<CChar>?) -> CInt
    let insert_marker: @convention(c) (Double, CInt, CInt, UnsafePointer<CChar>?) -> CInt
    let add_streamer: @convention(c) (UnsafePointer<CChar>?, CInt, CInt, UnsafePointer<CChar>?) -> CInt
    let delete_streamer: @convention(c) (UnsafePointer<CChar>?, CInt, CInt, UnsafePointer<CChar>?) -> CInt
    let release_all_sessions: @convention(c) () -> CInt
    let set_log_level_board_controller: @convention(c) (CInt) -> CInt
    let set_log_file_board_controller: @convention(c) (UnsafePointer<CChar>?) -> CInt
    let log_message_board_controller: @convention(c) (CInt, UnsafeMutablePointer<CChar>?) -> CInt
    let get_version_board_controller: VersionFunction

    let get_board_descr: BoardInfoStringFunction
    let get_sampling_rate: BoardInfoIntFunction
    let get_package_num_channel: BoardInfoIntFunction
    let get_timestamp_channel: BoardInfoIntFunction
    let get_marker_channel: BoardInfoIntFunction
    let get_battery_channel: BoardInfoIntFunction
    let get_num_rows: BoardInfoIntFunction
    let get_eeg_names: BoardInfoStringFunction
    let get_exg_channels: BoardInfoChannelsFunction
    let get_eeg_channels: BoardInfoChannelsFunction
    let get_emg_channels: BoardInfoChannelsFunction
    let get_ecg_channels: BoardInfoChannelsFunction
    let get_eog_channels: BoardInfoChannelsFunction
    let get_ppg_channels: BoardInfoChannelsFunction
    let get_optical_channels: BoardInfoChannelsFunction
    let get_eda_channels: BoardInfoChannelsFunction
    let get_accel_channels: BoardInfoChannelsFunction
    let get_rotation_channels: BoardInfoChannelsFunction
    let get_analog_channels: BoardInfoChannelsFunction
    let get_gyro_channels: BoardInfoChannelsFunction
    let get_other_channels: BoardInfoChannelsFunction
    let get_temperature_channels: BoardInfoChannelsFunction
    let get_resistance_channels: BoardInfoChannelsFunction
    let get_magnetometer_channels: BoardInfoChannelsFunction
    let get_device_name: BoardInfoStringFunction
    let get_board_presets: @convention(c) (CInt, UnsafeMutablePointer<CInt>?, UnsafeMutablePointer<CInt>?) -> CInt

    private static let lock = NSLock()
    private static var cached: BoardShimNative?

    static func withBoard<T>(_ body: (BoardShimNative) throws -> T) throws -> T {
        try body(load())
    }

    private static func load() throws -> BoardShimNative {
        lock.lock()
        defer { lock.unlock() }
        if let cached { return cached }
        let value = try BoardShimNative(library: NativeLibraries.boardController.load())
        cached = value
        return value
    }

    private init(library: NativeLibrary) throws {
        prepare_session = try library.symbol("prepare_session", as: type(of: prepare_session))
        start_stream = try library.symbol("start_stream", as: type(of: start_stream))
        stop_stream = try library.symbol("stop_stream", as: type(of: stop_stream))
        release_session = try library.symbol("release_session", as: type(of: release_session))
        get_current_board_data = try library.symbol("get_current_board_data", as: type(of: get_current_board_data))
        get_board_data_count = try library.symbol("get_board_data_count", as: type(of: get_board_data_count))
        get_board_data = try library.symbol("get_board_data", as: type(of: get_board_data))
        get_board_sampling_rate = try library.symbol("get_board_sampling_rate", as: type(of: get_board_sampling_rate))
        config_board = try library.symbol("config_board", as: type(of: config_board))
        config_board_with_bytes = try library.symbol("config_board_with_bytes", as: type(of: config_board_with_bytes))
        is_prepared = try library.symbol("is_prepared", as: type(of: is_prepared))
        insert_marker = try library.symbol("insert_marker", as: type(of: insert_marker))
        add_streamer = try library.symbol("add_streamer", as: type(of: add_streamer))
        delete_streamer = try library.symbol("delete_streamer", as: type(of: delete_streamer))
        release_all_sessions = try library.symbol("release_all_sessions", as: type(of: release_all_sessions))
        set_log_level_board_controller = try library.symbol("set_log_level_board_controller", as: type(of: set_log_level_board_controller))
        set_log_file_board_controller = try library.symbol("set_log_file_board_controller", as: type(of: set_log_file_board_controller))
        log_message_board_controller = try library.symbol("log_message_board_controller", as: type(of: log_message_board_controller))
        get_version_board_controller = try library.symbol("get_version_board_controller", as: type(of: get_version_board_controller))
        get_board_descr = try library.symbol("get_board_descr", as: type(of: get_board_descr))
        get_sampling_rate = try library.symbol("get_sampling_rate", as: type(of: get_sampling_rate))
        get_package_num_channel = try library.symbol("get_package_num_channel", as: type(of: get_package_num_channel))
        get_timestamp_channel = try library.symbol("get_timestamp_channel", as: type(of: get_timestamp_channel))
        get_marker_channel = try library.symbol("get_marker_channel", as: type(of: get_marker_channel))
        get_battery_channel = try library.symbol("get_battery_channel", as: type(of: get_battery_channel))
        get_num_rows = try library.symbol("get_num_rows", as: type(of: get_num_rows))
        get_eeg_names = try library.symbol("get_eeg_names", as: type(of: get_eeg_names))
        get_exg_channels = try library.symbol("get_exg_channels", as: type(of: get_exg_channels))
        get_eeg_channels = try library.symbol("get_eeg_channels", as: type(of: get_eeg_channels))
        get_emg_channels = try library.symbol("get_emg_channels", as: type(of: get_emg_channels))
        get_ecg_channels = try library.symbol("get_ecg_channels", as: type(of: get_ecg_channels))
        get_eog_channels = try library.symbol("get_eog_channels", as: type(of: get_eog_channels))
        get_ppg_channels = try library.symbol("get_ppg_channels", as: type(of: get_ppg_channels))
        get_optical_channels = try library.symbol("get_optical_channels", as: type(of: get_optical_channels))
        get_eda_channels = try library.symbol("get_eda_channels", as: type(of: get_eda_channels))
        get_accel_channels = try library.symbol("get_accel_channels", as: type(of: get_accel_channels))
        get_rotation_channels = try library.symbol("get_rotation_channels", as: type(of: get_rotation_channels))
        get_analog_channels = try library.symbol("get_analog_channels", as: type(of: get_analog_channels))
        get_gyro_channels = try library.symbol("get_gyro_channels", as: type(of: get_gyro_channels))
        get_other_channels = try library.symbol("get_other_channels", as: type(of: get_other_channels))
        get_temperature_channels = try library.symbol("get_temperature_channels", as: type(of: get_temperature_channels))
        get_resistance_channels = try library.symbol("get_resistance_channels", as: type(of: get_resistance_channels))
        get_magnetometer_channels = try library.symbol("get_magnetometer_channels", as: type(of: get_magnetometer_channels))
        get_device_name = try library.symbol("get_device_name", as: type(of: get_device_name))
        get_board_presets = try library.symbol("get_board_presets", as: type(of: get_board_presets))
    }
}

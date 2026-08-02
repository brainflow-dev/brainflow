import Foundation

public final class MLModel {
    private let params: BrainFlowModelParams
    private let serialized_params: String

    public init(params: BrainFlowModelParams) throws {
        self.params = params
        serialized_params = try params.to_json()
    }

    public static func set_log_level(_ log_level: Int) throws {
        try MLModelNative.withML { native in
            try checkBrainFlowExitCode(native.set_log_level_ml_module(CInt(log_level)), "Error in set_log_level")
        }
    }

    public static func set_log_level(_ log_level: LogLevels) throws {
        try set_log_level(log_level.rawValue)
    }

    public static func enable_ml_logger() throws {
        try set_log_level(LogLevels.LEVEL_INFO)
    }

    public static func enable_dev_ml_logger() throws {
        try set_log_level(LogLevels.LEVEL_TRACE)
    }

    public static func disable_ml_logger() throws {
        try set_log_level(LogLevels.LEVEL_OFF)
    }

    public static func set_log_file(_ log_file: String) throws {
        try log_file.withCString { path in
            try MLModelNative.withML { native in
                try checkBrainFlowExitCode(native.set_log_file_ml_module(path), "Error in set_log_file")
            }
        }
    }

    public static func log_message(_ log_level: Int, message: String) throws {
        var mutableMessage = Array(message.utf8CString)
        try mutableMessage.withUnsafeMutableBufferPointer { pointer in
            try MLModelNative.withML { native in
                try checkBrainFlowExitCode(native.log_message_ml_module(CInt(log_level), pointer.baseAddress), "Error in log_message")
            }
        }
    }

    public static func release_all() throws {
        try MLModelNative.withML { native in
            try checkBrainFlowExitCode(native.release_all(), "Error in release_all")
        }
    }

    public static func get_version() throws -> String {
        let maxLength = 64
        var bytes = [CChar](repeating: 0, count: maxLength)
        var length: CInt = 0
        try bytes.withUnsafeMutableBufferPointer { pointer in
            try MLModelNative.withML { native in
                try checkBrainFlowExitCode(native.get_version_ml_module(pointer.baseAddress, &length, CInt(maxLength)), "Error in get_version")
            }
        }
        return String(bytes: bytes.prefix(Int(length)).map { UInt8(bitPattern: $0) }, encoding: .utf8) ?? ""
    }

    public func prepare() throws {
        try serialized_params.withCString { paramsPtr in
            try MLModelNative.withML { native in
                try checkBrainFlowExitCode(native.prepare(paramsPtr), "Error in prepare")
            }
        }
    }

    public func release() throws {
        try serialized_params.withCString { paramsPtr in
            try MLModelNative.withML { native in
                try checkBrainFlowExitCode(native.release(paramsPtr), "Error in release")
            }
        }
    }

    public func predict(input_data: [Double]) throws -> [Double] {
        var input = input_data
        var output = [Double](repeating: 0.0, count: params.max_array_size)
        var outputLen: CInt = 0
        try serialized_params.withCString { paramsPtr in
            try input.withUnsafeMutableBufferPointer { inputPtr in
                try output.withUnsafeMutableBufferPointer { outputPtr in
                    try MLModelNative.withML { native in
                        try checkBrainFlowExitCode(native.predict(inputPtr.baseAddress, CInt(input_data.count), outputPtr.baseAddress, &outputLen, paramsPtr), "Error in predict")
                    }
                }
            }
        }
        return Array(output.prefix(Int(outputLen)))
    }
}

final class MLModelNative {
    typealias VersionFunction = @convention(c) (UnsafeMutablePointer<CChar>?, UnsafeMutablePointer<CInt>?, CInt) -> CInt

    let prepare: @convention(c) (UnsafePointer<CChar>?) -> CInt
    let predict: @convention(c) (UnsafeMutablePointer<Double>?, CInt, UnsafeMutablePointer<Double>?, UnsafeMutablePointer<CInt>?, UnsafePointer<CChar>?) -> CInt
    let release: @convention(c) (UnsafePointer<CChar>?) -> CInt
    let release_all: @convention(c) () -> CInt
    let set_log_level_ml_module: @convention(c) (CInt) -> CInt
    let set_log_file_ml_module: @convention(c) (UnsafePointer<CChar>?) -> CInt
    let log_message_ml_module: @convention(c) (CInt, UnsafeMutablePointer<CChar>?) -> CInt
    let get_version_ml_module: VersionFunction

    private static let lock = NSLock()
    private static var cached: MLModelNative?

    static func withML<T>(_ body: (MLModelNative) throws -> T) throws -> T {
        try body(load())
    }

    private static func load() throws -> MLModelNative {
        lock.lock()
        defer { lock.unlock() }
        if let cached { return cached }
        let value = try MLModelNative(library: NativeLibraries.mlModule.load())
        cached = value
        return value
    }

    private init(library: NativeLibrary) throws {
        prepare = try library.symbol("prepare", as: type(of: prepare))
        predict = try library.symbol("predict", as: type(of: predict))
        release = try library.symbol("release", as: type(of: release))
        release_all = try library.symbol("release_all", as: type(of: release_all))
        set_log_level_ml_module = try library.symbol("set_log_level_ml_module", as: type(of: set_log_level_ml_module))
        set_log_file_ml_module = try library.symbol("set_log_file_ml_module", as: type(of: set_log_file_ml_module))
        log_message_ml_module = try library.symbol("log_message_ml_module", as: type(of: log_message_ml_module))
        get_version_ml_module = try library.symbol("get_version_ml_module", as: type(of: get_version_ml_module))
    }
}

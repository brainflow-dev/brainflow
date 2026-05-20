import Foundation

#if os(Linux)
import Glibc
#else
import Darwin
#endif

final class NativeLibrary {
    private let handle: UnsafeMutableRawPointer

    init(names: [String]) throws {
        var errors = [String]()
        for path in Self.candidatePaths(for: names) {
            if let handle = dlopen(path, Self.openFlags) {
                self.handle = handle
                return
            }
            if let error = dlerror().map({ String(cString: $0) }) {
                errors.append("\(path): \(error)")
            }
        }
        throw BrainFlowError(
            "Unable to load BrainFlow native library. Set BRAINFLOW_LIB_DIR or build native libs into installed/lib. Tried: \(errors.joined(separator: "; "))",
            BrainFlowExitCodes.GENERAL_ERROR.rawValue
        )
    }

    deinit {
        dlclose(handle)
    }

    func symbol<T>(_ name: String, as type: T.Type) throws -> T {
        guard let pointer = dlsym(handle, name) else {
            let message = dlerror().map { String(cString: $0) } ?? "symbol not found"
            throw BrainFlowError("Unable to load symbol \(name): \(message)", BrainFlowExitCodes.GENERAL_ERROR.rawValue)
        }
        return unsafeBitCast(pointer, to: type)
    }

    private static var openFlags: Int32 {
        return RTLD_NOW | RTLD_GLOBAL
    }

    private static func candidatePaths(for names: [String]) -> [String] {
        var dirs = [String]()
        let env = ProcessInfo.processInfo.environment

        if let explicit = env["BRAINFLOW_LIB_DIR"], !explicit.isEmpty {
            dirs.append(explicit)
        }
        dirs.append(contentsOf: splitPathList(env["DYLD_LIBRARY_PATH"]))
        dirs.append(contentsOf: splitPathList(env["LD_LIBRARY_PATH"]))

        let cwd = FileManager.default.currentDirectoryPath
        dirs.append(cwd)
        dirs.append("\(cwd)/installed/lib")
        dirs.append("\(cwd)/../installed/lib")
        dirs.append("\(cwd)/../../installed/lib")
        dirs.append("\(cwd)/lib")

        #if os(macOS) || os(iOS)
        if let privateFrameworksPath = Bundle.main.privateFrameworksPath {
            dirs.append(privateFrameworksPath)
        }
        if let resourcePath = Bundle.main.resourcePath {
            dirs.append(resourcePath)
            dirs.append("\(resourcePath)/lib")
            dirs.append("\(resourcePath)/Frameworks")
        }
        #endif

        var candidates = [String]()
        for dir in unique(dirs) {
            for name in names {
                candidates.append((dir as NSString).appendingPathComponent(name))
            }
        }
        candidates.append(contentsOf: names)
        return unique(candidates)
    }

    private static func splitPathList(_ value: String?) -> [String] {
        guard let value, !value.isEmpty else { return [] }
        return value.split(separator: ":").map(String.init)
    }

    private static func unique(_ values: [String]) -> [String] {
        var seen = Set<String>()
        var result = [String]()
        for value in values where !value.isEmpty && !seen.contains(value) {
            seen.insert(value)
            result.append(value)
        }
        return result
    }
}

enum NativeLibraries {
    static let boardController = LazyNativeLibrary(names: [
        platformLibraryName(base: "BoardController"),
        "BoardController"
    ])
    static let dataHandler = LazyNativeLibrary(names: [
        platformLibraryName(base: "DataHandler"),
        "DataHandler"
    ])
    static let mlModule = LazyNativeLibrary(names: [
        platformLibraryName(base: "MLModule"),
        "MLModule"
    ])

    private static func platformLibraryName(base: String) -> String {
        #if os(Windows)
        return "\(base).dll"
        #elseif os(macOS) || os(iOS)
        return "lib\(base).dylib"
        #else
        return "lib\(base).so"
        #endif
    }
}

final class LazyNativeLibrary {
    private let names: [String]
    private let lock = NSLock()
    private var storage: NativeLibrary?

    init(names: [String]) {
        self.names = names
    }

    func load() throws -> NativeLibrary {
        lock.lock()
        defer { lock.unlock() }
        if let storage {
            return storage
        }
        let library = try NativeLibrary(names: names)
        storage = library
        return library
    }
}

import Foundation

/// BoardDescription formalizes the attributes of a BrainFlow board via a structured JSON object.
struct BoardDescription: Codable, Equatable {
    var package_num_channel: Int32 = 0
    var timestamp_channel: Int32 = 0
    var accel_channels: [Int32] = [Int32]()
    var ecg_channels: [Int32] = [Int32]()
    var eeg_channels: [Int32] = [Int32]()
    var eeg_names: String = ""
    var emg_channels: [Int32] = [Int32]()
    var eog_channels: [Int32] = [Int32]()
    var marker_channel: Int32 = 0
    var name: String = ""
    var num_rows: Int32 = 0
    var sampling_rate: Int32 = 0
    var battery_channel: Int32 = 0
    var eda_channels: [Int32] = [Int32]()
    var gyro_channels: [Int32] = [Int32]()
    var ppg_channels: [Int32] = [Int32]()
    var resistance_channels: [Int32] = [Int32]()
    var temperature_channels: [Int32] = [Int32]()
        
    // decode the input JSON into self:
    init(_ descriptionJSON: String?) throws {
        guard let thisJSON = descriptionJSON else {
            return
        }
        
        let decoder = JSONDecoder()
        let jsonData = Data(thisJSON.utf8)
        
        do {
            let json = try decoder.decode(type(of: self), from: jsonData)
            self = json
        } catch {
            try? BoardShim.logMessage(.LEVEL_CRITICAL, "board description JSON decoding error:\n \(error)")
            throw BrainFlowError("Invalid board description JSON", .NO_SUCH_DATA_IN_JSON_ERROR)
        }
    }
    
    /// Encode BoardDescription object into a JSON string.
    static func toJSON(_ boardDescription: Self) -> String {
        let encoder = JSONEncoder()
        encoder.outputFormatting = .prettyPrinted
        if let data = try? encoder.encode(boardDescription),
            let jsonString = String(data: data, encoding: .utf8) {
            return jsonString
        } else {
            return ""
        }
    }
    
    /// Decode the JSON string into a BoardDescription object.
    static func fromJSON(_ descriptionJSON: String?) throws -> Self {
        return try BoardDescription(descriptionJSON)
    }
    
}

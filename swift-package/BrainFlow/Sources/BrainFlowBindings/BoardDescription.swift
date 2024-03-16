//
//  BoardDescription.swift
//  a Swift reimagining of BrainFlow's board_description data type
//
//  Created by Scott Miller for Aeris Rising, LLC on 8/23/21.
//
import Foundation
//import BrainFlow

struct BoardDescription: Codable, Equatable {
    let package_num_channel: Int32
    let timestamp_channel: Int32
    let accel_channels: [Int32]?
    let ecg_channels: [Int32]?
    let eeg_channels: [Int32]?
    let eeg_names: String?
    let emg_channels: [Int32]?
    let eog_channels: [Int32]?
    let marker_channel: Int32?
    let name: String?
    let num_rows: Int32?
    let sampling_rate: Int32?
    let battery_channel: Int32?
    let eda_channels: [Int32]?
    let gyro_channels: [Int32]?
    let ppg_channels: [Int32]?
    let resistance_channels: [Int32]?
    let temperature_channels: [Int32]?
    
    // decode the input JSON into self:
    init(_ descriptionJSON: String) throws {
        guard descriptionJSON != "" else {
            throw BrainFlowException("Nil board description JSON", .JSON_NOT_FOUND_ERROR)
        }
        
        let decoder = JSONDecoder()
        let jsonData = Data(descriptionJSON.utf8)
        
        do {
            let json = try decoder.decode(type(of: self), from: jsonData)
            self = json
        } catch {
            try? BoardShim.logMessage(.LEVEL_CRITICAL, "board description JSON decoding error:\n \(error)")
            throw BrainFlowException("Invalid board description JSON", .NO_SUCH_DATA_IN_JSON_ERROR)
        }
    }
}

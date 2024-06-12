//
//  BrainFlowExtensions.swift
//  helper functions for the Swift binding of BrainFlow's API
//
//  Created by Scott Miller for Aeris Rising, LLC on 8/25/21.
//

import Foundation

extension Array {
    /// Convert a 1D array into a 2D matrix.
    func matrix2D(rowLength: Int) -> [[Element]] {
        return stride(from: 0, to: count, by: rowLength).map {
            Array(self[$0 ..< Swift.min($0 + rowLength, count)])
        }
    }
}

extension Array where Element == CChar {
    ///  Convert an array of characters returned from a C++ API, into a String.
    func toString(_ len: Int32) -> String {
        let data = Data(bytes: self, count: Int(len))
        if let result = String(data: data, encoding: .utf8) {
            return result }
        else {
            return "*** Array.toString: INVALID [CChar] ***"
        }
    }
}

extension String {
    func convertToDictionary() -> [String: Any]? {
        if let data = data(using: .utf8) {
            return try? JSONSerialization.jsonObject(with: data, options: []) as? [String: Any]
        }
        return nil
    }
}

extension Encodable {
    // Convert the current struct to a JSON string:
    func encodeJSON() throws -> String {
        let encoder = JSONEncoder()
        encoder.outputFormatting = .prettyPrinted.union(.withoutEscapingSlashes)
        
        do {
            let data = try encoder.encode(self)
            if let str = String(data: data, encoding: .utf8) {
                return str
            } else {
                return ""
            }
        } catch {
            throw BrainFlowError("Invalid JSON", .NO_SUCH_DATA_IN_JSON_ERROR)
        }
    }
}


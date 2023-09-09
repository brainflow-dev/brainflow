//
//  BrainFlowInputParams.swift
//  a Swift binding for BrainFlow's brainflow_input_params.h
//
//  Created by Scott Miller for Aeris Rising, LLC on 8/23/21.
//

import Foundation
//import BrainFlow

struct BrainFlowInputParams: Encodable {
    var serial_port: String = ""
    var mac_address: String = ""
    var ip_address: String = ""
    var ip_port: Int = 0
    var ip_protocol: Int = 0
    var other_info: String = ""
    var timeout: Int = 0
    var serial_number: String = ""
    var file: String = ""

    // Convert the BrainFlowInputParams struct to a JSON string:
    func json() -> String {
        let encoder = JSONEncoder()
        encoder.outputFormatting = .prettyPrinted.union(.withoutEscapingSlashes)

        guard let jsonParams = try? encoder.encode(self) else {
            try? BoardShim.logMessage(.LEVEL_ERROR, "Cannot convert BrainFlowInputParams to JSON")
            return ""
        }

        let stringParams = String(data: jsonParams, encoding: .utf8)
        return stringParams!
    }
}



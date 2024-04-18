//
//  eeg_metrics.swift
//  eeg_metrics
//
//  Created by Scott Miller on 4/8/24.
//

import Foundation
import BrainFlow
import ArgumentParser

@main
struct eeg_metrics: ParsableCommand {
    @Option(name: [.long], help: "timeout for device discovery or connection")
    var timeout: Int = 0
    @Option(name: [.long, .customLong("ip-port")], help: "ip port")
    var ip_port: Int = 0
    @Option(name: [.long, .customLong("ip-protocol")], help: "ip protocol, check IpProtocolType enum")
    var ip_protocol: Int = 0
    @Option(name: [.long, .customLong("ip-address")], help: "ip address")
    var ip_address: String = ""
    @Option(name: [.long, .customLong("serial-port")], help: "serial port")
    var serial_port: String = ""
    @Option(name: [.long, .customLong("mac-address")], help: "mac address")
    var mac_address: String = ""
    @Option(name: [.long, .customLong("other-info")], help: "other info")
    var other_info: String = ""
    @Option(name: [.long, .customLong("serial-number")], help: "serial number")
    var serial_number: String = ""
    @Option(name: [.long, .customLong("board-id")], help: "board id, check docs to get a list of supported boards")
    var board_id: Int32 = -1
    @Option(name: [.long], help: "file")
    var file: String = ""
    @Option(name: [.long, .customLong("master-board")], help: "master board id for streaming and playback boards")
    var master_board: Int = -1

    mutating func run() throws {
        var params = BrainFlowInputParams()
        params.ip_port = ip_port
        params.serial_port = serial_port
        params.mac_address = mac_address
        params.other_info = other_info
        params.serial_number = serial_number
        params.ip_address = ip_address
        params.ip_protocol = ip_protocol
        params.timeout = timeout
        params.file = file
        params.master_board = master_board
        guard let boardId = BoardIds(rawValue: board_id) else {
            try? BoardShim.logMessage (.LEVEL_ERROR, "Invalid board ID: \(board_id)")
            return
        }
        
        try BoardShim.enableBoardLogger()
        try DataFilter.enableDataLogger()
        try MLModule.enableMLlogger()

        let board = try BoardShim(boardId, params)
        let masterBoardId = try board.getBoardId()
        let samplingRate = try BoardShim.getSamplingRate(masterBoardId)
        try board.prepareSession()
        try board.startStream(bufferSize: 45000)
        try BoardShim.logMessage(.LEVEL_INFO, "start sleeping in the main thread")
        sleep(5)  // recommended window size for eeg metric calculation is at least 4 seconds, bigger is better
        let size = try board.getBoardDataCount()
        let data = try board.getBoardData(size)
        try board.stopStream()
        try board.releaseSession()

        let EEGchannels = try BoardShim.getEEGchannels(masterBoardId)

        // avg band power
        let bands = try DataFilter.getAvgBandPowers(data: data, channels: EEGchannels,
                                                    samplingRate: samplingRate, applyFilter: true)
        let avgFeatureVector = bands.0 + bands.1
        print("testEEGmetrics->avg featureVector: \(avgFeatureVector)")

        // custom band power
        let avgBands = [(2.0, 4.0), (4.0, 8.0), (8.0, 13.0), (13.0, 30.0), (30.0, 45.0)]
        let newBands = try DataFilter.getCustomBandPowers(data: data, bands: avgBands, channels: EEGchannels, samplingRate: samplingRate, applyFilter: true)
        let featureVector = newBands.0 + newBands.1
        print("testEEGmetrics->custom featureVector: \(featureVector)")

        // calc concentration
        let concentrationParams = BrainFlowModelParams(metric: .MINDFULNESS, classifier: .DEFAULT_CLASSIFIER)
        let concentration = MLModule(modelParams: concentrationParams)
        try concentration.prepareClassifier()
        let concClass = try concentration.predictClass(data: featureVector)
        print("testEEGmetrics->concClass: \(concClass)")
        try concentration.releaseClassifier()

        // restfulness + onnx is not supported:
        let restfulnessParams = BrainFlowModelParams(metric: .RESTFULNESS, classifier: .DEFAULT_CLASSIFIER)
        let restfulness = MLModule(modelParams: restfulnessParams)
        try restfulness.prepareClassifier()
        let restClass = try restfulness.predictClass(data: featureVector)
        print("testEEGmetrics->restClass: \(restClass)")
        try restfulness.releaseClassifier()
    }
}




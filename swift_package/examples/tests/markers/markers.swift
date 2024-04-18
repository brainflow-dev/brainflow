//
//  markers.swift
//  markers
//
//  Created by Scott Miller on 4/8/24.
//

import Foundation
import BrainFlow
import ArgumentParser

@main
struct markers: ParsableCommand {
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
        
        try BoardShim.enableDevBoardLogger()
        let board = try BoardShim(boardId, params)
        try board.prepareSession()
        let boardDescription = try BoardShim.getBoardDescr(board.boardId)

        try board.startStream(bufferSize: 45000)
        for i in 0..<10 {
            sleep(1)
            try board.insertMarker(value: Double(i + 1)) }
        let size = try board.getBoardDataCount()
        let data = try board.getBoardData(size)
        try board.stopStream()
        try board.releaseSession()

        if let markerChInt32 = boardDescription.marker_channel {
            let markerCh = Int(markerChInt32)
            let uqMarkers = Set(data[markerCh])
            print("Data:\n\(data)")
            print("Unique markers: \(uqMarkers)")
        } else {
            print("ERROR: nil marker channel")
        }
    }
}



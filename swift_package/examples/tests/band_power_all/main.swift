//
//  main.swift
//  band_power_all
//
//  Created by Scott Miller on 4/12/24.
//

import Foundation

func testBandPowerAll() throws {
    // use synthetic board for demo
    try BoardShim.enableDevBoardLogger()
    let params = BrainFlowInputParams()
    let boardId = BoardIds.SYNTHETIC_BOARD
    let boardDescription = try BoardShim.getBoardDescr(boardId)
    guard let samplingRate = boardDescription.sampling_rate else {
        try? BoardShim.logMessage(.LEVEL_ERROR, "null sampling rate in board description")
        return
    }
    let board = try BoardShim(boardId, params)
    try board.prepareSession()
    try board.startStream()
    try BoardShim.logMessage(.LEVEL_INFO, "start sleeping in the main thread")
    sleep(5)
    let nfft = try DataFilter.getNearestPowerOfTwo(samplingRate)
    var data = try board.getBoardData()
    try board.stopStream()
    try board.releaseSession()

    let eegChannels = try BoardShim.getEEGchannels(boardId)
    let bands = try DataFilter.getAvgBandPowers(data: data, channels: eegChannels, samplingRate: samplingRate, applyFilter: true)
    print("avg band powers : \(bands.0)")
    print("stddev band powers : \(bands.1)")
}

try? testBandPowerAll()

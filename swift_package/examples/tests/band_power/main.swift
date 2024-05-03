//
//  main.swift
//  band_power
//
//  Created by Scott Miller on 4/12/24.
//

import Foundation

func testBandPower() throws {
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
    let size = try board.getBoardDataCount()
    var data = try board.getBoardData(size)
    try board.stopStream()
    try board.releaseSession()

    guard let EEGchannels = boardDescription.eeg_channels else {
        try? BoardShim.logMessage(.LEVEL_ERROR, "null EEG channels in board description")
        return
    }
    // second eeg channel of synthetic board is a sine wave at 10Hz, should see huge alpha
    let eegChannel = Int(EEGchannels[1])
    // optional detrend
    try DataFilter.deTrend(data: &data[eegChannel], operation: .LINEAR)
    
    let overlap = Int32(floor(Double(Int(nfft) / 2)))
    let psd = try DataFilter.getPSDwelch(data: data[eegChannel], nfft: nfft,
                                     overlap: overlap, samplingRate: samplingRate,
                                     window: .BLACKMAN_HARRIS)

    let bandPowerAlpha = try DataFilter.getBandPower(psd: psd, freqStart: 7.0, freqEnd: 13.0)
    let bandPowerBeta = try DataFilter.getBandPower(psd: psd, freqStart: 14.0, freqEnd: 30.0)
    print("alpha/beta:\(bandPowerAlpha / bandPowerBeta)")
}

try? testBandPower()

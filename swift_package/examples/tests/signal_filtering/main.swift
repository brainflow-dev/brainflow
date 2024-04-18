//
//  main.swift
//  signal_filtering
//
//  Created by Scott Miller on 4/13/24.
//

import Foundation
import BrainFlow

func testSignalFiltering() throws {
    try BoardShim.enableDevBoardLogger()
    let params = BrainFlowInputParams()
    let board = try BoardShim(.SYNTHETIC_BOARD, params)
    try board.prepareSession()
    try board.startStream()
    try BoardShim.logMessage(.LEVEL_INFO, "start sleeping in the main thread")
    sleep(10)
    let size = try board.getBoardDataCount()
    var data = try board.getBoardData(size)
    try board.stopStream()
    try board.releaseSession()
    let samplingRate = try BoardShim.getSamplingRate(board.boardId)
    let EEGchannels = try BoardShim.getEEGchannels(board.boardId)
    
    // for demo apply different filters to different channels, in production choose one
    for count in EEGchannels.indices {
        let channel = Int(EEGchannels[count])
        let beforeSum = Double(data[channel].compactMap( {$0} ).reduce(0, +))

        // filters work in-place
        switch count {
        case 0:
            try DataFilter.performBandpass(data: &data[channel],  samplingRate: samplingRate,
                                           startFreq: 12.0, stopFreq: 18.0, order: 4,
                                           filterType: .BESSEL, ripple: 0)
        case 1:
            try DataFilter.performBandstop(data: &data[channel],  samplingRate: samplingRate,
                                           startFreq: 29.0, stopFreq: 31.0 , order: 3,
                                           filterType: .BUTTERWORTH, ripple: 0)
        case 2:
            try DataFilter.performLowpass(data: &data[channel],  samplingRate: samplingRate,
                                          cutoff: 20.0, order: 5,
                                          filterType: .CHEBYSHEV_TYPE_1, ripple: 1)
        case 3:
            try DataFilter.performHighpass(data: &data[channel],  samplingRate: samplingRate,
                                           cutoff: 3.0, order: 4,
                                           filterType: .BUTTERWORTH, ripple: 0)
        case 4:
            try DataFilter.performRollingFilter(data: &data[channel], period: 3,
                                                operation: .MEAN)
        default:
            try DataFilter.removeEnvironmentalNoise(data: &data[channel],
                                                    samplingRate: samplingRate, noiseType: .FIFTY)
        }
        
        let afterSum = Double(data[channel].compactMap{$0}.reduce(0, +))
    }
}

try? testSignalFiltering()

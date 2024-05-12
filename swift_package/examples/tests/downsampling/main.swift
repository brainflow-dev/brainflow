//
//  main.swift
//  downsampling
//
//  Created by Scott Miller on 4/12/24.
//

import Foundation

func testDownsampleData() throws {
    try BoardShim.enableDevBoardLogger()
    let params = BrainFlowInputParams()
    let board = try BoardShim(.SYNTHETIC_BOARD, params)
    try board.prepareSession()
    try board.startStream()
    try BoardShim.logMessage(.LEVEL_INFO, "start sleeping in the main thread")
    sleep(10)
    let data = try board.getBoardData(20)
    try board.stopStream()
    try board.releaseSession()

    let EEGchannels = try BoardShim.getEEGchannels(.SYNTHETIC_BOARD)
    // demo for downsampling, it just aggregates data
    for count in EEGchannels.indices {
        let channel = Int(EEGchannels[count])
        print("Original data for channel \(channel)")
        print(data[channel])
        var downsampledData = [Double]()
        let beforeSum = Double(data[channel].compactMap( {$0} ).reduce(0, +))

        switch count {
        case 0:
            downsampledData = try DataFilter.performDownsampling(data: data[channel], period: 3, operation: .MEDIAN)
        case 1:
            downsampledData = try DataFilter.performDownsampling(data: data[channel], period: 2, operation: .MEAN)
        default:
            downsampledData = try DataFilter.performDownsampling(data: data[channel], period: 2, operation: .EACH)
        }

        let afterSum = Double(downsampledData.compactMap{$0}.reduce(0, +))
        print("Downsampled data for channel \(channel)")
        print("before sum: \(beforeSum), after sum: \(afterSum)")
        print(downsampledData)
    }
}

try? testDownsampleData()

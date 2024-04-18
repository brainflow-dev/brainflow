//
//  main.swift
//  ica
//
//  Created by Scott Miller on 4/13/24.
//

import Foundation
import BrainFlow

// From https://stackoverflow.com/questions/48088882/how-can-split-from-string-to-array-by-chunks-of-given-size
extension Array {
    func chunks(size: Int) -> [[Element]] {
        return stride(from: 0, to: count, by: size).map {
            Array(self[$0 ..< Swift.min($0 + size, count)])
        }
    }
}

// From https://stackoverflow.com/questions/39889568/how-to-transpose-an-array-more-swiftly
func matrixTranspose<T>(_ matrix: [[T]]) -> [[T]] {
    if matrix.isEmpty {return matrix}
    var result = [[T]]()
    for index in 0..<matrix.first!.count {
        result.append(matrix.map{$0[index]})
    }
    return result
}

func testICA() throws {
    let params = BrainFlowInputParams();
    let boardShim = try BoardShim(.SYNTHETIC_BOARD, params)
    let preset = BrainFlowPresets.DEFAULT_PRESET
    try boardShim.prepareSession()
    try boardShim.startStream(bufferSize: 45000, streamerParams: "")
    sleep(10)
    try boardShim.stopStream()
    let data = try boardShim.getBoardData(500, preset)
    try boardShim.releaseSession()

    let eegChannels = try BoardShim.getEEGchannels(.SYNTHETIC_BOARD, preset)
    let selectedChannel = Int(eegChannels[4])
    let originalData = data[selectedChannel]
    let transposedReshapedData = matrixTranspose(originalData.chunks(size: 5))
    let tuple = try DataFilter.performICA(data: transposedReshapedData, numComponents: 2)
    print("result:\n\(tuple)")
}

try? testICA()

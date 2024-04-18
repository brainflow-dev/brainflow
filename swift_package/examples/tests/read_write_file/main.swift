//
//  main.swift
//  read_write_file
//
//  Created by Scott Miller on 4/12/24.
//

import Foundation
import BrainFlow

func testReadWriteFile() throws {
    try BoardShim.enableDevBoardLogger()
    let params = BrainFlowInputParams()
    let board = try BoardShim(.SYNTHETIC_BOARD, params)
    try board.prepareSession()
    try board.startStream()
    try BoardShim.logMessage(.LEVEL_INFO, "start sleeping in the main thread")
    sleep(10)
    let size = try board.getBoardDataCount()
    let data = try board.getBoardData(size)
    try board.stopStream()
    try board.releaseSession()

    // demo how to convert it to pandas DF and plot data
    _ = try BoardShim.getEEGchannels(.SYNTHETIC_BOARD)
    print("Data From the Board")
    print(Array(data[..<10]))

    // demo for data serialization using brainflow API, we recommend to use it instead pandas.to_csv()
    try DataFilter.writeFile(data: data, fileName: "test.csv", fileMode: "w")  // use 'a' for append mode
    
    let restoredData = try DataFilter.readFile(fileName: "test.csv")
    print("Data From the File")
    print(Array(restoredData))
}

try? testReadWriteFile()

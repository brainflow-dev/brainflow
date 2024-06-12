//
//  main.swift
//  denoising
//
//  Created by Scott Miller on 4/13/24.
//

import Foundation

@main
struct testDenoising {
    static func main() throws {
        try BoardShim.enableDevBoardLogger()
        let params = BrainFlowInputParams()
        let board = try BoardShim(.SYNTHETIC_BOARD, params)
        try board.prepareSession()
        try board.startStream()
        try BoardShim.logMessage(.LEVEL_INFO, "start sleeping in the main thread")
        sleep(20)
        let size = try board.getBoardDataCount()
        var data = try board.getBoardData(size)
        try board.stopStream()
        try board.releaseSession()
    
        let EEGchannels = try BoardShim.getEEGchannels(board.boardId)
    
        // demo for denoising, apply different methods to different channels for demo
        for count in EEGchannels.indices {
            let channel = Int(EEGchannels[count])
            let beforeSum = Double(data[channel].compactMap( {$0} ).reduce(0, +))
            let beforeShape = (data.count, data[0].count)
    
            // first of all you can try simple moving median or moving average with different window size
            switch count {
            case 0:
                try DataFilter.performRollingFilter(data: &data[channel], period: 3, operation: .MEAN)
                let afterSum = Double(data[channel].compactMap{$0}.reduce(0, +))
            case 1:
                try DataFilter.performRollingFilter(data: &data[channel], period: 3, operation: .MEDIAN)
                let afterSum = Double(data[channel].compactMap{$0}.reduce(0, +))
            // if methods above dont work for your signal you can try wavelet based denoising
            // feel free to try different functions and decomposition levels
            case 2:
                try DataFilter.performWaveletDenoising (data: &data[channel], wavelet: .DB6, decompositionLevel: 3)
                let afterSum = Double(data[channel].compactMap{$0}.reduce(0, +))
            case 3:
                try DataFilter.performWaveletDenoising(data: &data[channel], wavelet: .BIOR3_9,
                                                       decompositionLevel: 3)
                let afterSum = Double(data[channel].compactMap{$0}.reduce(0, +))
            case 4:
                try DataFilter.performWaveletDenoising(data: &data[channel], wavelet: .SYM7,
                                                       decompositionLevel: 3)
                let afterSum = Double(data[channel].compactMap{$0}.reduce(0, +))
            case 5:
                // with synthetic board this one looks like the best option, but it depends on many circumstances
                try DataFilter.performWaveletDenoising(data: &data[channel], wavelet: .COIF3,
                                                       decompositionLevel: 3)
                let afterSum = Double(data[channel].compactMap{$0}.reduce(0, +))
            default:
                print("Skipping channel \(channel)")
            }
            
            let afterShape = (data.count, data[0].count)
        }
    }
}

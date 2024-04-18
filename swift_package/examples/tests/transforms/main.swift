//
//  main.swift
//  transforms
//
//  Created by Scott Miller on 4/13/24.
//

import Foundation
import BrainFlow

func testTransforms() throws {
    try BoardShim.enableDevBoardLogger()
    let params = BrainFlowInputParams()
    let board = try BoardShim(.SYNTHETIC_BOARD, params)
    try board.prepareSession()
    let samplingRate = try BoardShim.getSamplingRate(board.boardId)
    try board.startStream()
    try BoardShim.logMessage(.LEVEL_INFO, "start sleeping in the main thread")
    sleep(10)
    let data = try board.getCurrentBoardData(DataFilter.getNearestPowerOfTwo(samplingRate))
    try board.stopStream()
    try board.releaseSession()

    let EEGchannels = try BoardShim.getEEGchannels(board.boardId)
    // demo for transforms
    for count in EEGchannels.indices {
        let channel = Int(EEGchannels[count])
        print("Original data for channel \(channel)")
        print(data[channel])
        // demo for wavelet transforms
        //wavelet_coeffs format is[A(J) D(J) D(J-1) ..... D(1)] where J is decomposition level, A - app coeffs, D - detailed coeffs
        // lengths array stores lengths for each block
        let (waveletCoeffs, lengths) = try DataFilter.performWaveletTransform(data: data[channel], wavelet: .DB5, decompositionLevel: 3)
        //let appCoefs = Array(waveletCoeffs[0..<Int(lengths[0])])
        //let detailedCoeffsFirstBlock = Array(waveletCoeffs[Int(lengths[0])..<Int(lengths[1])])
        // you can do smth with wavelet coeffs here, for example denoising works via thresholds
        // for wavelets coefficients
        let restoredData = try DataFilter.performInverseWaveletTransform(
                                            waveletTuple: (waveletCoeffs, lengths),
                                            originalDataLen: Int32(data[channel].count),
                                            wavelet: .DB5, decompositionLevel: 3)
        print("Restored data after wavelet transform for channel \(channel):")
        print(restoredData)

        // demo for fft, len of data must be a power of 2
        let FFTdata = try DataFilter.performFFT(data: data[channel], window: .NO_WINDOW)
        // len of fft_data is N / 2 + 1
        let restoredFFTdata = try DataFilter.performIFFT(data: FFTdata)
        print("Restored data after fft for channel \(channel):")
        print(restoredFFTdata)
    }
}

try? testTransforms()

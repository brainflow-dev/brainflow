//
//  BrainFlowCItests.swift
//  functions for Continuous Integration (CI) testing of the Swift Bindings for BrainFlow

//  Created by Scott Miller for Aeris Rising, LLC on 9/23/21.


import Foundation
import XCTest
import BrainFlow

class BrainFlowCItests: XCTestCase {
    func testGetDataFromBoard() throws {
        do {
            try BoardShim.enableDevBoardLogger()
            let params = BrainFlowInputParams()
            let board = try BoardShim(.SYNTHETIC_BOARD, params)
            try board.prepareSession()
            
            try board.startStream(bufferSize: 45000)
            sleep(10)
            let size = try board.getBoardDataCount()
            let data = try board.getBoardData(size)  // get all data and remove it from internal buffer
            try board.stopStream()
            try board.releaseSession()
            
            XCTAssert((data.count == 32) && (data[0].count > 100))
            print(data)
        } catch let bfError as BrainFlowException {
            try? BoardShim.logMessage (.LEVEL_ERROR, bfError.message)
            try? BoardShim.logMessage (.LEVEL_ERROR, "Error code: \(bfError.errorCode)")
            XCTFail()
        }
    }
    
    func testMarkers() throws {
        try BoardShim.enableDevBoardLogger()
        let params = BrainFlowInputParams()
        let board = try BoardShim(.SYNTHETIC_BOARD, params)
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
            XCTAssert(uqMarkers.count == 10)
            print(data)
        } else {
            XCTAssert(false)
        }
    }
    
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
        
        for channel in data.indices {
            XCTAssert(compareData(from: data[channel], to: restoredData[channel], tol: 1e-3)) }
    }
    
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
            XCTAssert(beforeSum != afterSum)
            print("Downsampled data for channel \(channel)")
            print(downsampledData)
        }
    }
    
    private func compareData(from: [Double], to: [Double], tol: Double) -> Bool {
        // return true if all elements match to within given tolerance
        guard ((from.count > 0) && (from.indices == to.indices)) else {
            print("compareData: sizes mismatch")
            return false
        }
        
        for i in from.indices {
            if abs(from[i] - to[i]) > tol {
                return false
            }
        }
    
        return true
    }
    
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
            
            XCTAssert(compareData(from: data[channel], to: restoredData, tol: 1e-5))
            XCTAssert(compareData(from: data[channel], to: restoredFFTdata, tol: 1e-5))
        }
    }
    
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
        
        // demo how to convert it to pandas DF and plot data
        //df = pd.DataFrame(np.transpose(data))
        //plt.figure()
        //df[eeg_channels].plot(subplots=True)
        //plt.savefig('before_processing.png')

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
            XCTAssert(beforeSum != afterSum)
        }
    }
    
    func testDenoising() throws {
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
                XCTAssert((beforeSum != afterSum))
            case 1:
                try DataFilter.performRollingFilter(data: &data[channel], period: 3, operation: .MEDIAN)
                let afterSum = Double(data[channel].compactMap{$0}.reduce(0, +))
                XCTAssert((beforeSum != afterSum))
            // if methods above dont work for your signal you can try wavelet based denoising
            // feel free to try different functions and decomposition levels
            case 2:
                try DataFilter.performWaveletDenoising (data: &data[channel], wavelet: .DB6, decompositionLevel: 3)
                let afterSum = Double(data[channel].compactMap{$0}.reduce(0, +))
                XCTAssert((beforeSum != afterSum))
            case 3:
                try DataFilter.performWaveletDenoising(data: &data[channel], wavelet: .BIOR3_9,
                                                       decompositionLevel: 3)
                let afterSum = Double(data[channel].compactMap{$0}.reduce(0, +))
                XCTAssert((beforeSum != afterSum))
            case 4:
                try DataFilter.performWaveletDenoising(data: &data[channel], wavelet: .SYM7,
                                                       decompositionLevel: 3)
                let afterSum = Double(data[channel].compactMap{$0}.reduce(0, +))
                XCTAssert((beforeSum != afterSum))
            case 5:
                // with synthetic board this one looks like the best option, but it depends on many circumstances
                try DataFilter.performWaveletDenoising(data: &data[channel], wavelet: .COIF3,
                                                       decompositionLevel: 3)
                let afterSum = Double(data[channel].compactMap{$0}.reduce(0, +))
                XCTAssert((beforeSum != afterSum))
            default:
                print("Skipping channel \(channel)")
            }
            
            let afterShape = (data.count, data[0].count)
            XCTAssert(beforeShape == afterShape)
        }
    }
    
    func testBandPower() throws {
        // use synthetic board for demo
        try BoardShim.enableDevBoardLogger()
        let params = BrainFlowInputParams()
        let boardId = BoardIds.SYNTHETIC_BOARD
        let boardDescription = try BoardShim.getBoardDescr(boardId)
        guard let samplingRate = boardDescription.sampling_rate else {
            XCTAssert(false)
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
            XCTAssert(false)
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

        // fail test if ratio is not smth we expect
        XCTAssert((bandPowerAlpha / bandPowerBeta) >= 100.0)
    }
    
    func testEEGmetrics() throws {
        try BoardShim.enableBoardLogger()
        try DataFilter.enableDataLogger()
        try MLModule.enableMLlogger()

        let params = BrainFlowInputParams()
        let board = try BoardShim(.SYNTHETIC_BOARD, params)
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
        XCTAssert(avgFeatureVector.count == 10)
        XCTAssert((avgFeatureVector.min()! >= 0.0) && (avgFeatureVector.max()! <= 2.0))

        // custom band power
        let avgBands = [(2.0, 4.0), (4.0, 8.0), (8.0, 13.0), (13.0, 30.0), (30.0, 45.0)]
        let newBands = try DataFilter.getCustomBandPowers(data: data, bands: avgBands, channels: EEGchannels, samplingRate: samplingRate, applyFilter: true)
        let featureVector = newBands.0 + newBands.1
        print("testEEGmetrics->custom featureVector: \(featureVector)")
        XCTAssert(featureVector.count == 10)
        XCTAssert((featureVector.min()! >= 0.0) && (featureVector.max()! <= 2.0))

        // calc concentration
        let concentrationParams = BrainFlowModelParams(metric: .MINDFULNESS, classifier: .DEFAULT_CLASSIFIER)
        let concentration = MLModule(modelParams: concentrationParams)
        try concentration.prepareClassifier()
        let concClass = try concentration.predictClass(data: featureVector)
        print("testEEGmetrics->concClass: \(concClass)")
        try concentration.releaseClassifier()
        XCTAssert(concClass.count == 1)
        XCTAssert((concClass.min()! >= 0.0) && (concClass.max()! <= 1.0))

        // restfulness + onnx is not supported:
        let relaxationParams = BrainFlowModelParams(metric: .RESTFULNESS, classifier: .ONNX_CLASSIFIER)
        let relaxation = MLModule(modelParams: relaxationParams)
        XCTAssertThrowsError(try relaxation.prepareClassifier())
    }
        
}

//
//  BrainFlowCItests.swift
//  functions for Continuous Integration (CI) testing of the Swift Bindings for BrainFlow

//  Created by Scott Miller on 9/23/21.


import Foundation


import XCTest
@testable import BCILab

class BrainFlowCItests: XCTestCase {
    let cytonBoardID = BoardIds.CYTON_BOARD
    let synthBoardID = BoardIds.SYNTHETIC_BOARD
    let boardParams = BrainFlowInputParams(serial_port: "/dev/cu.usbserial-DM0258EJ")

    func testGetDataFromBoard() throws {
        try BoardShim.enableDevBoardLogger()
        let params = BrainFlowInputParams()
        let board = try BoardShim(.SYNTHETIC_BOARD, params)
        try board.prepareSession()

        // board.start_stream () # use this for default options
        try board.startStream(bufferSize: 45000)
        sleep(10)
        // data = board.get_current_board_data (256) # get latest 256 packages or less, doesnt remove them from internal buffer
        let data = try board.getBoardData()  // get all data and remove it from internal buffer
        try board.stopStream()
        try board.releaseSession()

        XCTAssert((data.count == 32) && (data[0].count > 100))
        print(data)
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
        let data = try board.getBoardData()
        try board.stopStream()
        try board.releaseSession()

        let markerCh = Int(boardDescription.marker_channel)
        let  uqMarkers = Set(data[markerCh])
        XCTAssert((data.count == 32) && (data[0].count > 100) && (uqMarkers.count == 10))
        print(data)
    }
    
    func testReadWriteFile() throws {
        try BoardShim.enableDevBoardLogger()
        let params = BrainFlowInputParams()
        let board = try BoardShim(.SYNTHETIC_BOARD, params)
        try board.prepareSession()
        try board.startStream()
        try BoardShim.logMessage(.LEVEL_INFO, "start sleeping in the main thread")
        sleep(10)
        let data = try board.getBoardData()
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
            let (waveletCoeffs, lengths) = try DataFilter.performWaveletTransform(data: data[channel], wavelet: "db5", decompositionLevel: 3)
            //let appCoefs = Array(waveletCoeffs[0..<Int(lengths[0])])
            //let detailedCoeffsFirstBlock = Array(waveletCoeffs[Int(lengths[0])..<Int(lengths[1])])
            // you can do smth with wavelet coeffs here, for example denoising works via thresholds
            // for wavelets coefficients
            let restoredData = try DataFilter.performInverseWaveletTransform(
                                                waveletTuple: (waveletCoeffs, lengths),
                                                originalDataLen: Int32(data[channel].count),
                                                wavelet: "db5", decompositionLevel: 3)
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
        var data = try board.getBoardData()
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
                                               centerFreq: 15.0, bandWidth: 6.0, order: 4,
                                               filterType: .BESSEL, ripple: 0)
            case 1:
                try DataFilter.performBandstop(data: &data[channel],  samplingRate: samplingRate,
                                               centerFreq: 30.0, bandWidth: 1.0, order: 3,
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
        
        //df = pd.DataFrame(np.transpose(data))
        //plt.figure()
        //df[eeg_channels].plot(subplots=True)
        //plt.savefig('after_processing.png')
    }
    
    func testDenoising() throws {
        try BoardShim.enableDevBoardLogger()
        let params = BrainFlowInputParams()
        let board = try BoardShim(.SYNTHETIC_BOARD, params)
        try board.prepareSession()
        try board.startStream()
        try BoardShim.logMessage(.LEVEL_INFO, "start sleeping in the main thread")
        sleep(20)
        var data = try board.getBoardData()
        try board.stopStream()
        try board.releaseSession()

        // demo how to convert it to pandas DF and plot data
        let EEGchannels = try BoardShim.getEEGchannels(board.boardId)
        
        //df = pd.DataFrame(np.transpose(data))
        //plt.figure()
        //df[eeg_channels].plot(subplots=True)
        //plt.savefig('before_processing.png')

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
                try DataFilter.performWaveletDenoising(data: &data[channel], wavelet: "db6",
                                                       decompositionLevel: 3)
                let afterSum = Double(data[channel].compactMap{$0}.reduce(0, +))
                XCTAssert((beforeSum != afterSum))
            case 3:
                try DataFilter.performWaveletDenoising(data: &data[channel], wavelet: "bior3.9",
                                                       decompositionLevel: 3)
                let afterSum = Double(data[channel].compactMap{$0}.reduce(0, +))
                XCTAssert((beforeSum != afterSum))
            case 4:
                try DataFilter.performWaveletDenoising(data: &data[channel], wavelet: "sym7",
                                                       decompositionLevel: 3)
                let afterSum = Double(data[channel].compactMap{$0}.reduce(0, +))
                XCTAssert((beforeSum != afterSum))
            case 5:
                // with synthetic board this one looks like the best option, but it depends on many circumstances
                try DataFilter.performWaveletDenoising(data: &data[channel], wavelet: "coif3",
                                                       decompositionLevel: 3)
                let afterSum = Double(data[channel].compactMap{$0}.reduce(0, +))
                XCTAssert((beforeSum != afterSum))
            default:
                print("Skipping channel \(channel)")
            }
            
            let afterShape = (data.count, data[0].count)
            XCTAssert(beforeShape == afterShape)
        }
        
        //df = pd.DataFrame(np.transpose(data))
        //plt.figure()
        //df[eeg_channels].plot(subplots=True)
        //plt.savefig('after_processing.png')
    }
    
    func testBandPower() throws {
        // use synthetic board for demo
        try BoardShim.enableDevBoardLogger()
        let params = BrainFlowInputParams()
        let boardId = BoardIds.SYNTHETIC_BOARD
        let boardDescription = try BoardShim.getBoardDescr(boardId)
        let samplingRate = boardDescription.sampling_rate
        let board = try BoardShim(boardId, params)
        try board.prepareSession()
        try board.startStream()
        try BoardShim.logMessage(.LEVEL_INFO, "start sleeping in the main thread")
        sleep(5)
        let nfft = try DataFilter.getNearestPowerOfTwo(samplingRate)
        var data = try board.getBoardData()
        try board.stopStream()
        try board.releaseSession()

        let EEGchannels = boardDescription.eeg_channels
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
        let data = try board.getBoardData()
        try board.stopStream()
        try board.releaseSession()

        let EEGchannels = try BoardShim.getEEGchannels(masterBoardId)
        let bands = try DataFilter.getAvgBandPowers(data: data, channels: EEGchannels,
                                                    samplingRate: samplingRate, applyFilters: true)
        let featureVector = bands.0 + bands.1
        print(featureVector)

        // calc concentration
        let concentrationParams = BrainFlowModelParams(metric: .CONCENTRATION, classifier: .KNN)
        let concentration = MLModule(modelParams: concentrationParams)
        try concentration.prepareClassifier()
        let concClass = try concentration.predictClass(data: featureVector)
        print("Concentration: \(concClass)")
        try concentration.releaseClassifier()

        // calc relaxation
        let relaxationParams = BrainFlowModelParams(metric: .RELAXATION, classifier: .REGRESSION)
        let relaxation = MLModule(modelParams: relaxationParams)
        try relaxation.prepareClassifier()
        let relaxClass = try relaxation.predictClass(data: featureVector)
        print("Relaxation: \(relaxClass)")
        try relaxation.releaseClassifier()
    }
        
}

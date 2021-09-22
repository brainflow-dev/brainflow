//
//  DataFilterTests.swift
//  These are the unit tests for DataFilter.swift.  They are modeled after
//  https://github.com/brainflow-dev/brainflow/tree/master/tests/python

import XCTest
@testable import BCILab

class DataFilterTests: XCTestCase {
    
    func testBandPower() {
        BoardShim.enableDevBoardLogger()

        // use synthetic board for demo
        do {
            let params = BrainFlowInputParams()
            let boardId = BoardIds.SYNTHETIC_BOARD
            let boardDescription = try BoardShim.getBoardDescr(boardId)
            let samplingRate = boardDescription.sampling_rate
            let board = try BoardShim(boardId, params)
            try board.prepareSession()
            try board.startStream()
            try BoardShim.logMessage(logLevel: .LEVEL_INFO, message: "start sleeping in the main thread")
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
            XCTAssert((bandPowerAlpha / bandPowerBeta) >= 100.0) }
        catch let bfError as BrainFlowException {
            try? BoardShim.logMessage (logLevel: .LEVEL_ERROR, message: bfError.message)
            try? BoardShim.logMessage (logLevel: .LEVEL_ERROR, message: "Error code: \(bfError.errorCode)")
            }
        catch {
            try? BoardShim.logMessage (logLevel: .LEVEL_ERROR, message: "undefined exception")
        }
    }
    
    func testBandPowerAll() {
        BoardShim.enableDevBoardLogger()

        // use synthetic board for demo
        let params = BrainFlowInputParams()
        let boardId = BoardIds.SYNTHETIC_BOARD
        do {
            let samplingRate = try BoardShim.getSamplingRate(boardId)
            let board = try BoardShim(boardId, params)
            try board.prepareSession()
            try board.startStream()
            try BoardShim.logMessage(logLevel: .LEVEL_INFO, message: "start sleeping in the main thread")
            sleep(5)
            let data = try board.getBoardData()
            try board.stopStream()
            try board.releaseSession()

            let EEGchannels = try BoardShim.getEEGchannels(boardId)
            let bands = try DataFilter.getAvgBandPowers(data: data, channels: EEGchannels,
                                                        samplingRate: samplingRate, applyFilters: true)
            
            let avgSum = bands.0.reduce(0, +)
            let stdSum = bands.1.reduce(0, +)
            XCTAssert((bands.0.count == 5) && (bands.1.count == 5) &&
                      (avgSum > 0) && (avgSum <= 1) && (stdSum > 0) && (stdSum < 10))
        }
        catch let bfError as BrainFlowException {
            try? BoardShim.logMessage (logLevel: .LEVEL_ERROR, message: bfError.message)
            try? BoardShim.logMessage (logLevel: .LEVEL_ERROR, message: "Error code: \(bfError.errorCode)")
            }
        catch {
            try? BoardShim.logMessage (logLevel: .LEVEL_ERROR, message: "undefined exception")
        }
    }
    
    func testCSP() {
        let labels: [Double] = [0.0, 1.0]
        let data: [[[Double]]] = [[[6, 3, 1, 5], [3, 0, 5, 1]], [[1, 5, 6, 2], [5, 1, 2, 2]]]
        let trueFilters: [[String]] = [["-0.313406", "0.079215"], ["-0.280803", "-0.480046"]]
        let trueEigVals: [String] = ["0.456713", "0.752979"]

        do {
            let (filters, eigVals) = try DataFilter.getCSP(data: data, labels: labels)
            
            let roundFilters = filters.map( { $0.map( {String(format: "%.6f", $0)}) } )
            let roundEigVals = eigVals.map( {String(format: "%.6f", $0)} )
            
            XCTAssert(roundFilters == trueFilters)
            XCTAssert(roundEigVals == trueEigVals)
        }
        catch let bfError as BrainFlowException {
            try? BoardShim.logMessage (logLevel: .LEVEL_ERROR, message: bfError.message)
            try? BoardShim.logMessage (logLevel: .LEVEL_ERROR, message: "Error code: \(bfError.errorCode)")
            }
        catch {
            try? BoardShim.logMessage (logLevel: .LEVEL_ERROR, message: "undefined exception")
        }
    }
    
    func stdDev(_ data : [Double]) -> Double
    {
        let len = Double(data.count)
        let mean = data.reduce(0, {$0 + $1}) / len
        let sumOfSq = data.map { pow($0 - mean, 2.0)}.reduce(0, {$0 + $1})
        return sqrt(sumOfSq / len)
    }
    
    func testDenoising() {
        // use synthetic board for demo
        let params = BrainFlowInputParams()
        let boardId = BoardIds.SYNTHETIC_BOARD
  
        do {
            let board = try BoardShim(boardId, params)
            try board.prepareSession()
            try board.startStream()
            try BoardShim.logMessage(logLevel: .LEVEL_INFO, message: "start sleeping in the main thread")
            sleep(5)
            var data = try board.getBoardData()
            try board.stopStream()
            try board.releaseSession()
            
            // demo how to convert it to pandas DF and plot data
            let EEGchannels = try BoardShim.getEEGchannels(boardId)

            // demo for denoising, apply different methods to different channels for demo
            for count in EEGchannels.indices {
                // first of all you can try simple moving median or moving average with different window size
                let channel = Int(EEGchannels[count])
                let beforeSum = Double(data[channel].compactMap( {$0} ).reduce(0, +))
                
                switch count {
                case 0:
                    try DataFilter.performRollingFilter(data: &data[channel], period: 3, operation: .MEAN)
                case 1:
                    try DataFilter.performRollingFilter(data: &data[channel], period: 3, operation: .MEDIAN)
                    // if methods above dont work for your signal you can try wavelet based denoising
                    // feel free to try different functions and decomposition levels
                case 2:
                    try DataFilter.performWaveletDenoising(data: &data[channel], wavelet: "db6", decompositionLevel: 3)
                case 3:
                    try DataFilter.performWaveletDenoising(data: &data[channel], wavelet: "bior3.9", decompositionLevel: 3)
                case 4:
                    try DataFilter.performWaveletDenoising(data: &data[channel], wavelet: "sym7", decompositionLevel: 3)
                case 5:
                    // with synthetic board this one looks like the best option, but it depends on many circumstances
                    try DataFilter.performWaveletDenoising(data: &data[channel], wavelet: "coif3", decompositionLevel: 3)
                default:
                    throw BrainFlowException("Invalid channel value: \(channel)", .EMPTY_BUFFER_ERROR)
                }

                let afterSum = Double(data[channel].compactMap{$0}.reduce(0, +))
                XCTAssert((beforeSum != afterSum))
            }
        }
        catch let bfError as BrainFlowException {
            try? BoardShim.logMessage (logLevel: .LEVEL_ERROR, message: bfError.message)
            try? BoardShim.logMessage (logLevel: .LEVEL_ERROR, message: "Error code: \(bfError.errorCode)")
            }
        catch {
            try? BoardShim.logMessage (logLevel: .LEVEL_ERROR, message: "undefined exception")
        }
    }
    
    func testDownsampling () {
        BoardShim.enableDevBoardLogger()
        // use synthetic board for demo
        let params = BrainFlowInputParams()
        let boardId = BoardIds.SYNTHETIC_BOARD
        var downsampledData: [Double]
        
        do {
            // use synthetic board for demo
            let board = try BoardShim(boardId, params)
            try board.prepareSession()
            try board.startStream()
            try BoardShim.logMessage(logLevel: .LEVEL_INFO, message: "start sleeping in the main thread")
            sleep(5)
            let data = try board.getBoardData(20)
            try board.stopStream()
            try board.releaseSession()

            let EEGchannels = try BoardShim.getEEGchannels(.SYNTHETIC_BOARD)
            // demo for downsampling, it just aggregates data
            for count in EEGchannels.indices {
                let channel = Int(EEGchannels[count])
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
            }
        }
        catch let bfError as BrainFlowException {
            try? BoardShim.logMessage (logLevel: .LEVEL_ERROR, message: bfError.message)
            try? BoardShim.logMessage (logLevel: .LEVEL_ERROR, message: "Error code: \(bfError.errorCode)")
            }
        catch {
            try? BoardShim.logMessage (logLevel: .LEVEL_ERROR, message: "undefined exception")
        }
    }
    
    func testSignalFiltering() {
        BoardShim.enableDevBoardLogger()
        // use synthetic board for demo
        let params = BrainFlowInputParams()
        let boardId = BoardIds.SYNTHETIC_BOARD

        do {
            // use synthetic board for demo
            let board = try BoardShim(boardId, params)
            let samplingRate = try BoardShim.getSamplingRate(boardId)
            try board.prepareSession()
            try board.startStream()
            try BoardShim.logMessage(logLevel: .LEVEL_INFO, message: "start sleeping in the main thread")
            sleep(5)
            var data = try board.getBoardData()
            try board.stopStream()
            try board.releaseSession()

            // demo how to convert it to pandas DF and plot data
            let EEGchannels = try BoardShim.getEEGchannels(boardId)

            // for demo apply different filters to different channels, in production choose one
            for count in EEGchannels.indices {
                let channel = Int(EEGchannels[count])
                let beforeSum = Double(data[channel].compactMap( {$0} ).reduce(0, +))

                // filters work in-place
                switch count {
                case 0:
                    try DataFilter.performBandpass(data: &data[channel], samplingRate: samplingRate,
                                                   centerFreq: 15.0, bandWidth: 6.0, order: 4,
                                                   filterType: .BESSEL, ripple: 0)
                case 1:
                    try DataFilter.performBandstop(data: &data[channel], samplingRate: samplingRate,
                                                   centerFreq: 30.0, bandWidth: 1.0, order: 3,
                                                   filterType: .BUTTERWORTH, ripple: 0)
                case 2:
                    try DataFilter.performLowpass(data: &data[channel], samplingRate: samplingRate,
                                                  cutoff: 20.0, order: 5, filterType: .CHEBYSHEV_TYPE_1,
                                                  ripple: 1)
                case 3:
                    try DataFilter.performHighpass(data: &data[channel], samplingRate: samplingRate,
                                                   cutoff: 3.0, order: 4, filterType: .BUTTERWORTH, ripple: 0)
                case 4:
                    try DataFilter.performRollingFilter(data: &data[channel], period: 3, operation: .MEAN)
                default:
                    try DataFilter.removeEnvironmentalNoise(data: &data[channel], samplingRate: BoardShim.getSamplingRate(boardId), noiseType: .FIFTY)
                }
                
                let afterSum = Double(data[channel].compactMap{$0}.reduce(0, +))
                XCTAssert(beforeSum != afterSum)
            }
        }
        catch let bfError as BrainFlowException {
            try? BoardShim.logMessage (logLevel: .LEVEL_ERROR, message: bfError.message)
            try? BoardShim.logMessage (logLevel: .LEVEL_ERROR, message: "Error code: \(bfError.errorCode)")
            }
        catch {
            try? BoardShim.logMessage (logLevel: .LEVEL_ERROR, message: "undefined exception")
        }
    }
    
    // round to the 5th decimal place before comparing each item in the two arrays
    func compareFFT(_ preData: [Double], _ postData: [Double]) {
        for i in preData.indices {
            let format = "%.5f"
            let preString = String(format: format, preData[i])
            let postString = String(format: format, postData[i])
            
            XCTAssert(preString == postString)
        }
    }
    
    func testTransforms() {
        BoardShim.enableDevBoardLogger()
        // use synthetic board for demo
        let params = BrainFlowInputParams()
        let boardId = BoardIds.SYNTHETIC_BOARD

        do {
            // use synthetic board for demo
            let board = try BoardShim(boardId, params)
            let samplingRate = try BoardShim.getSamplingRate(boardId)
            let boardId = BoardIds.SYNTHETIC_BOARD
            try board.prepareSession()
            try board.startStream()
            try BoardShim.logMessage(logLevel: .LEVEL_INFO, message: "start sleeping in the main thread")
            sleep(5)
            let data = try board.getCurrentBoardData(DataFilter.getNearestPowerOfTwo(samplingRate))
            try board.stopStream()
            try board.releaseSession()

            let EEGchannels = try BoardShim.getEEGchannels(boardId)
            // demo for transforms
            for count in EEGchannels.indices {
                let channel = Int(EEGchannels[count])
                // demo for wavelet transforms
                // wavelet_coeffs format is[A(J) D(J) D(J-1) ..... D(1)] where J is decomposition level, A - app coeffs, D - detailed coeffs
                // lengths array stores lengths for each block
                let (waveletCoeffs, lengths) = try DataFilter.performWaveletTransform(
                                                data: data[channel], wavelet: "db5", decompositionLevel: 3)
                //let appCoefs = waveletCoeffs[0..<Int(lengths[0])]
                //let detailedCoeffsFirstBlock = waveletCoeffs[Int(lengths[0])..<Int(lengths[1])]
                // you can do smth with wavelet coeffs here, for example denoising works via thresholds
                // for wavelets coefficients
                let restoredData = try DataFilter.performInverseWaveletTransform(
                    waveletTuple: (waveletCoeffs, lengths), originalDataLen: Int32(data[channel].count),
                    wavelet: "db5", decompositionLevel: 3)
                compareFFT(data[channel], restoredData)
 
                // demo for fft, len of data must be a power of 2
                let fftData = try DataFilter.performFFT(data: data[channel], window: .NO_WINDOW)
                // len of fft_data is N / 2 + 1
                let restoredFFTdata = try DataFilter.performIFFT(data: fftData)
                
                compareFFT(data[channel], restoredFFTdata)
                //XCTAssert(fftData == restoredFFTdata)
            }
        }
        catch let bfError as BrainFlowException {
            try? BoardShim.logMessage (logLevel: .LEVEL_ERROR, message: bfError.message)
            try? BoardShim.logMessage (logLevel: .LEVEL_ERROR, message: "Error code: \(bfError.errorCode)")
            }
        catch {
            try? BoardShim.logMessage (logLevel: .LEVEL_ERROR, message: "undefined exception")
        }
    }
    
    func testWindowing () {
        let windowLen: Int32 = 20
        let testException = BrainFlowException("test message", .INVALID_ARGUMENTS_ERROR)
        do {
            for window in WindowFunctions.allCases {
                let windowData = try DataFilter.getWindow(window: window, windowLen: windowLen)
                XCTAssert(windowData.count == windowLen)
                XCTAssertThrowsError(try DataFilter.getWindow(window: window, windowLen: -1)) { error in
                    if let bfError = error as? BrainFlowException {
                        XCTAssertEqual(bfError, testException)
                    }
                }
            }
        }
        catch let bfError as BrainFlowException {
            try? BoardShim.logMessage (logLevel: .LEVEL_ERROR, message: bfError.message)
        }
        catch {
            try? BoardShim.logMessage (logLevel: .LEVEL_ERROR, message: "undefined exception")
        }
    }
}

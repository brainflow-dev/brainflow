//
//  DataFilterTests.swift
//  These are the unit tests for DataFilter.swift.  They are modeled after
//  https://github.com/brainflow-dev/brainflow/tree/master/tests/python

import XCTest
import BrainFlow

class DataFilterTests: XCTestCase {
    func testBandPowerAll() {
        // use synthetic board for demo
        let params = BrainFlowInputParams()
        let boardId = BoardIds.SYNTHETIC_BOARD
        do {
            try BoardShim.enableDevBoardLogger()
            let samplingRate = try BoardShim.getSamplingRate(boardId)
            let board = try BoardShim(boardId, params)
            try board.prepareSession()
            try board.startStream()
            try BoardShim.logMessage(.LEVEL_INFO, "start sleeping in the main thread")
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
            try? BoardShim.logMessage (.LEVEL_ERROR, bfError.message)
            try? BoardShim.logMessage (.LEVEL_ERROR, "Error code: \(bfError.errorCode)")
            }
        catch {
            try? BoardShim.logMessage (.LEVEL_ERROR, "undefined exception")
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
            try? BoardShim.logMessage (.LEVEL_ERROR, bfError.message)
            try? BoardShim.logMessage (.LEVEL_ERROR, "Error code: \(bfError.errorCode)")
            }
        catch {
            try? BoardShim.logMessage (.LEVEL_ERROR, "undefined exception")
        }
    }
    
    func stdDev(_ data : [Double]) -> Double
    {
        let len = Double(data.count)
        let mean = data.reduce(0, {$0 + $1}) / len
        let sumOfSq = data.map { pow($0 - mean, 2.0)}.reduce(0, {$0 + $1})
        return sqrt(sumOfSq / len)
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
    
    func testWindowing () {
        let windowLen: Int32 = 20
        let testError = BrainFlowException("test message", .INVALID_ARGUMENTS_ERROR)
        
        do {
            for window in WindowFunctions.allCases {
                let windowData = try DataFilter.getWindow(window: window, windowLen: windowLen)
                XCTAssert(windowData.count == windowLen)
                XCTAssertThrowsError(try DataFilter.getWindow(window: window, windowLen: -1)) { error in
                    if let bfError = error as? BrainFlowException {
                        XCTAssertEqual(bfError, testError)
                    }
                }
            }
        }
        catch let bfError as BrainFlowException {
            try? BoardShim.logMessage (.LEVEL_ERROR, bfError.message)
        }
        catch {
            try? BoardShim.logMessage (.LEVEL_ERROR, "undefined exception")
        }
    }
}

//
//  DataFilter.swift
//  A binding for BrainFlow's data_filter high-level API
//
//  Created by Scott Miller for Aeris Rising, LLC on 8/27/21.
//


import Foundation

struct DataFilter {

    /**
     * enable Data logger with level INFO
     */
    func enableDataLogger () throws {
        do {
            try setLogLevel (.LEVEL_INFO) }
        catch {
            throw error
        }
    }

    /**
     * enable Data logger with level TRACE
     */
    func enableDevDataLogger () throws {
        do {
            try setLogLevel (.LEVEL_TRACE) }
        catch {
            throw error
        }
    }

    /**
     * disable Data logger
     */
    func disableDataLogger () throws {
        do {
            try setLogLevel (.LEVEL_OFF) }
        catch {
            throw error
        }
    }

    /**
     * redirect logger from stderr to a file
     */
    func setLogFile (_ logFile: String) throws {
        var cLogFile = logFile.cString(using: String.Encoding.utf8)!
        let result = set_log_file (&cLogFile)
        try checkErrorCode(errorMsg: "Error in set_log_file", errorCode: result)
    }

    /**
     * set log level
     */
    private func setLogLevel (_ logLevel: LogLevels) throws {
        let result = set_log_level (logLevel.rawValue)
        try checkErrorCode(errorMsg: "Error in set_log_level", errorCode: result)
    }

    /**
    * perform lowpass filter in-place
    */
    func performLowpass (data: inout [Double], samplingRate: Int32, cutoff: Double,
                         order: Int32, filterType: FilterTypes, ripple: Double) throws {
        let dataCount = Int32(data.count)
        let filterVal = filterType.rawValue
        let result = perform_lowpass (&data, dataCount, samplingRate, cutoff, order, filterVal, ripple)
        try checkErrorCode(errorMsg: "Failed to apply filter", errorCode: result)
    }

    /**
    * perform highpass filter in-place
    */
    func performHighpass (data: inout [Double], samplingRate: Int32, cutoff: Double,
                         order: Int32, filterType: FilterTypes, ripple: Double) throws {
        let dataCount = Int32(data.count)
        let filterVal = filterType.rawValue
        let result = perform_highpass (&data, dataCount, samplingRate, cutoff, order, filterVal, ripple)
        try checkErrorCode(errorMsg: "Failed to apply filter", errorCode: result)
    }

    /**
     * perform bandpass filter in-place
     */
    func performBandpass (data: inout [Double], samplingRate: Int32, centerFreq: Double, bandWidth: Double,
                          order: Int32, filterType: FilterTypes, ripple: Double) throws {
        let dataCount = Int32(data.count)
        let filterVal = filterType.rawValue
        let result = perform_bandpass (&data, dataCount, samplingRate, centerFreq, bandWidth, order,
                                       filterVal, ripple)
        try checkErrorCode(errorMsg: "Failed to apply filter", errorCode: result)
    }

    /**
     * perform bandstop filter in-place
     */
    func performBandstop (data: inout [Double], samplingRate: Int32, centerFreq: Double, bandWidth: Double,
                          order: Int32, filterType: FilterTypes, ripple: Double) throws {
        let dataCount = Int32(data.count)
        let filterVal = filterType.rawValue
        let result = perform_bandstop (&data, dataCount, samplingRate, centerFreq, bandWidth, order,
                                       filterVal, ripple)
        try checkErrorCode(errorMsg: "Failed to apply filter", errorCode: result)
    }
    
    /**
     * perform moving average or moving median filter in-place
     */
    func performRollingFilter (data: inout [Double], period: Int32, operation: Int32) throws {
        let dataCount = Int32(data.count)
        let result = perform_rolling_filter (&data, dataCount, period, operation)
        try checkErrorCode(errorMsg: "Failed to apply filter", errorCode: result)
    }
    
    /**
     * removes noise using notch filter
     */
    func removeEnvironmentalNoise (data: inout [Double], samplingRate: Int32, noiseType: NoiseTypes) throws {
        let dataCount = Int32(data.count)
        let result = remove_environmental_noise (&data, dataCount, samplingRate, noiseType.rawValue)
        try checkErrorCode(errorMsg: "Failed to remove noise", errorCode: result)
    }

}

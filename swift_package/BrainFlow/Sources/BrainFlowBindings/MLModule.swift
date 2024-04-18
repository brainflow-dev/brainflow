//
//  MLModule.swift
//  a Swift binding for BrainFlow's MLModule API
//
//  Created by Scott Miller on 9/22/21 on behalf of Aeris Rising, LLC.
//

import Foundation

struct BrainFlowModelParams: Encodable {
    let metric: BrainFlowMetrics
    let classifier: BrainFlowClassifiers
    let file: String
    let other_info: String
    let output_name: String
    let max_array_size = 8192

    init (metric: BrainFlowMetrics, classifier: BrainFlowClassifiers,
          file: String = "", otherInfo: String = "", outputName: String = "") {
        self.metric = metric
        self.classifier = classifier
        self.file = file
        self.other_info = otherInfo
        self.output_name = outputName
    }
}

struct MLModule {
    //MLModule class used to calc derivative metrics from raw data
    
    let modelParams: BrainFlowModelParams
    
    static func setLogLevel(_ logLevel: LogLevels) throws {
        //set BrainFlow log level, use it only if you want to write your own messages to BrainFlow logger,
        //otherwise use enable_ml_logger, enable_dev_ml_logger or disable_ml_logger
        //:param log_level: log level, to specify it you should use values from LogLevels enum
        //:type log_level: int
        
        let errorCode = set_log_level_ml_module(logLevel.rawValue)
        try checkErrorCode("unable to enable logger", errorCode)
    }

    static func enableMLlogger() throws {
        //enable ML Logger with level INFO, uses stderr for log messages by default
        try setLogLevel(.LEVEL_INFO)
    }

    static func disableMLlogger() throws {
        ///disable BrainFlow Logger
        try setLogLevel(.LEVEL_OFF)
    }
    
    static func enableDevMLlogger() throws {
        //enable ML Logger with level TRACE, uses stderr for log messages by default
        try setLogLevel(.LEVEL_TRACE)
    }
    
    static func setLogFile(logFile: String) throws {
        //redirect logger from stderr to file, can be called any time
        //:param log_file: log file name
        //:type log_file: str
        
        var cFile = [CChar](repeating: CChar(0), count: 4096)
        let errorCode = set_log_file_ml_module(&cFile)
        try checkErrorCode("Cannot set log file to: \(logFile)", errorCode)
    }
        
    func prepareClassifier() throws {
        ///prepare classifier
        print("modelParams: \(modelParams)")
        let json = try self.modelParams.encodeJSON()
        var params = json.cString(using: .utf8)!
        let errorCode = prepare(&params)
        try checkErrorCode("Cannot prepare classifier", errorCode)
    }
    
    func releaseClassifier() throws {
        ///release classifier
        var params = try self.modelParams.encodeJSON().cString(using: .utf8)!
        let errorCode = release(&params)
        try checkErrorCode("Cannot release classifier", errorCode)
    }

    func predictClass(data: [Double]) throws -> [Double] {
        //calculate metric from data
        //:param data: input array
        //:type data: NDArray
        //:return: metric value
        //:rtype: float
        
        let len = Int32(data.count)
        var vData = data
        var output = [Double](repeating: 0.0, count: modelParams.max_array_size)
        var outputLen = [Int32](repeating: 0, count: 1)
        var params = try self.modelParams.encodeJSON().cString(using: .utf8)!
        let errorCode = predict(&vData, len, &output, &outputLen, &params)
        try checkErrorCode("unable to calc metric", errorCode)
        
        return Array(output[0..<Int(outputLen[0])])
    }
}

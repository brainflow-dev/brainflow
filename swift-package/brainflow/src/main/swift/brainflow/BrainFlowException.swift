//
//  BrainFlowException.swift
//  a Swift binding for BrainFlow's brainflow_exception.h
//
//  Created by Scott Miller on 8/23/21.
//

import Foundation

class BrainFlowException: Error, Equatable {
    static func == (lhs: BrainFlowException, rhs: BrainFlowException) -> Bool {
        return (lhs.errorCode == rhs.errorCode) 
    }
    
    var message: String
    var errorCode: BrainFlowExitCodes
    
    init(_ errorMessage: String, _ code: BrainFlowExitCodes) {
        message = errorMessage
        errorCode = code
    }
    
}

func checkErrorCode(_ errorMsg: String, _ errorCode: Int32) throws {
    if let bfErrorCode = BrainFlowExitCodes(rawValue: errorCode) {
        if bfErrorCode != BrainFlowExitCodes.STATUS_OK {
            throw BrainFlowException (errorMsg, bfErrorCode)
        }
    } else {
        throw BrainFlowException("Invalid error code: \(errorCode)", .UNKNOWN_CODE)
    }
}

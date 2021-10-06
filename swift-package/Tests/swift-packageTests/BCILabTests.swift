//
//  BCILabTests.swift
//  BCILabTests
//
//  Created by Scott Miller on 7/24/21.
//

import XCTest
@testable import BCILab

class BCILabTests: XCTestCase {
//    let cytonBoardID = BoardIds.CYTON_BOARD
//    let synthBoardID = BoardIds.SYNTHETIC_BOARD
//    let params = BrainFlowInputParams(serial_port: "/dev/cu.usbserial-DM0258EJ")
//
//    override func setUpWithError() throws {
//        // Put setup code here. This method is called before the invocation of each test method in the class.
//    }
//
//    override func tearDownWithError() throws {
//        // Put teardown code here. This method is called after the invocation of each test method in the class.
//    }
//
//    func testEEGnames() throws {
//        let result = try BoardShim.getEEGnames(boardId: synthBoardID)
//        XCTAssert(result == ["Fz", "C3", "Cz", "C4", "Pz", "PO7", "Oz", "PO8", "F5", "F7", "F3", "F1", "F2", "F4", "F6", "F8"])
//    }
//
//    func testSamplingRate() throws {
//        let result = try BoardShim.getSamplingRate(boardId: synthBoardID)
//        XCTAssert(result == 250)
//    }
//    
//    func testTimestampChannel () throws {
//        let result = try BoardShim.getTimestampChannel(boardId: synthBoardID)
//        XCTAssert(result == 30)
//    }
//
//    func testMarkerChannel () throws {
//        let result = try BoardShim.getMarkerChannel(boardId: synthBoardID)
//        XCTAssert(result == 31)
//    }
//    
//    func testNumRows () throws {
//        let result = try BoardShim.getNumRows(boardId: synthBoardID)
//        XCTAssert(result == 32)
//    }
//
//    func testPackageNumChannel () throws {
//        let result = try BoardShim.getPackageNumChannel(boardId: synthBoardID)
//        XCTAssert(result == 0)
//    }
//
//    func testBatteryChannel () throws {
//        let result = try BoardShim.getBatteryChannel(boardId: synthBoardID)
//        XCTAssert(result == 29)
//    }
//
//    func testBoardDescr () throws {
//        let result = try BoardShim.getBoardDescr(boardId: synthBoardID)
//        XCTAssert(result ==
//                    """
//                    {\"accel_channels\":[17,18,19],\"battery_channel\":29,\"ecg_channels\":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16],\"eda_channels\":[23],\"eeg_channels\":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16],\"eeg_names\":\"Fz,C3,Cz,C4,Pz,PO7,Oz,PO8,F5,F7,F3,F1,F2,F4,F6,F8\",\"emg_channels\":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16],\"eog_channels\":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16],\"gyro_channels\":[20,21,22],\"marker_channel\":31,\"name\":\"Synthetic\",\"num_rows\":32,\"package_num_channel\":0,\"ppg_channels\":[24,25],\"resistance_channels\":[27,28],\"sampling_rate\":250,\"temperature_channels\":[26],\"timestamp_channel\":30}
//                    """
//                    )
//    }
//
//    func testDeviceName () throws {
//        let result = try BoardShim.getDeviceName(boardId: synthBoardID)
//        XCTAssert(result == "Synthetic")
//    }
//
//    func testEEGchannels () throws {
//        let result = try BoardShim.getEEGchannels(boardId: synthBoardID)
//        XCTAssert(result == [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16])
//    }
//
//    func testEMGchannels () throws
//    {
//        let result = try BoardShim.getEMGchannels(boardId: synthBoardID)
//        XCTAssert(result == [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16])
//    }
//    
//    func testECGchannels () throws {
//        let result = try BoardShim.getECGchannels(boardId: synthBoardID)
//        XCTAssert(result == [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16])
//    }
//
//    func testTemperatureChannels () throws {
//        let result = try BoardShim.getTemperatureChannels(boardId: synthBoardID)
//        XCTAssert(result == [26])
//    }
//
//    func testResistanceChannels () throws {
//        let result = try BoardShim.getResistanceChannels(boardId: synthBoardID)
//        XCTAssert(result == [27,28])
//    }
//
//    func testEOGchannels () throws {
//        let result = try BoardShim.getEOGchannels(boardId: synthBoardID)
//        XCTAssert(result == [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16])
//    }
//
//    func testEXGchannels () throws {
//        let result = try BoardShim.getEXGchannels(boardId: synthBoardID)
//        XCTAssert(result == [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16])
//    }
//
//    func testEDAchannels () throws {
//        let result = try BoardShim.getEXGchannels(boardId: synthBoardID)
//        XCTAssert(result == [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16])
//    }
//
//    func testPPGchannels () throws {
//        let result = try BoardShim.getEXGchannels(boardId: synthBoardID)
//        XCTAssert(result == [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16])
//    }
//
//    func testAccelChannels () throws {
//        let result = try BoardShim.getAccelChannels(boardId: synthBoardID)
//        XCTAssert(result == [17,18,19])
//    }
//
//    func testGyroChannels () throws {
//        let result = try BoardShim.getGyroChannels(boardId: synthBoardID)
//        XCTAssert(result == [20,21,22])
//    }
//
//    func testAnalogChannels () throws {
//        XCTAssertThrowsError(try BoardShim.getAnalogChannels(boardId: synthBoardID), "test message") { error in
//            XCTAssertEqual(error as? BrainFlowException, BrainFlowException("Error in board info getter", .UNSUPPORTED_BOARD_ERROR))
//          }
//    }
//
//    func testOtherChannels () throws {
//        XCTAssertThrowsError(try BoardShim.getOtherChannels(boardId: synthBoardID), "test message") { error in
//            XCTAssertEqual(error as? BrainFlowException, BrainFlowException("Error in board info getter", .UNSUPPORTED_BOARD_ERROR))
//          }
//    }
//
//    func testMasterBoardID () throws {
//        let result = try BoardShim.getMasterBoardID(boardId: synthBoardID, params: params)
//        XCTAssert(result == .SYNTHETIC_BOARD)
//    }

    func testPerformanceExample() throws {
        // This is an example of a performance test case.
        self.measure {
            // Put the code you want to measure the time of here.
        }
    }

}

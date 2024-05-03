//
//  BCILabTests.swift
//  BCILabTests
//
//  Created by Scott Miller for Aeris Rising, LLC  on 7/24/21.
//

import XCTest
@testable import BrainFlow

class BoardShimTests: XCTestCase {
    let cytonBoardID = BoardIds.CYTON_BOARD
    let synthBoardID = BoardIds.SYNTHETIC_BOARD
    let params = BrainFlowInputParams(serial_port: "/dev/cu.usbserial-DM0258EJ")

    override func setUpWithError() throws {
        // Put setup code here. This method is called before the invocation of each test method in the class.
    }

    override func tearDownWithError() throws {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
    }

    func testEEGnames() throws {
        let result = try BoardShim.getEEGnames(synthBoardID)
        XCTAssert(result == ["Fz", "C3", "Cz", "C4", "Pz", "PO7", "Oz", "PO8", "F5", "F7", "F3", "F1", "F2", "F4", "F6", "F8"])
    }

    func testSamplingRate() throws {
        let result = try BoardShim.getSamplingRate(synthBoardID)
        XCTAssert(result == 250)
    }
    
    func testTimestampChannel () throws {
        let result = try BoardShim.getTimestampChannel(synthBoardID)
        XCTAssert(result == 30)
    }

    func testMarkerChannel () throws {
        let result = try BoardShim.getMarkerChannel(synthBoardID)
        XCTAssert(result == 31)
    }
    
    func testNumRows () throws {
        let result = try BoardShim.getNumRows(synthBoardID)
        XCTAssert(result == 32)
    }

    func testPackageNumChannel () throws {
        let result = try BoardShim.getPackageNumChannel(synthBoardID)
        XCTAssert(result == 0)
    }

    func testBatteryChannel () throws {
        let result = try BoardShim.getBatteryChannel(synthBoardID)
        XCTAssert(result == 29)
    }

    func testBoardDescr () throws {
        let result = try BoardShim.getBoardDescr(synthBoardID)
        print("result: \(result)")
        let testBoard = try BoardDescription(
                    """
                    {\"accel_channels\":[17,18,19],\"battery_channel\":29,\"ecg_channels\":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16],\"eda_channels\":[23],\"eeg_channels\":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16],\"eeg_names\":\"Fz,C3,Cz,C4,Pz,PO7,Oz,PO8,F5,F7,F3,F1,F2,F4,F6,F8\",\"emg_channels\":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16],\"eog_channels\":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16],\"gyro_channels\":[20,21,22],\"marker_channel\":31,\"name\":\"Synthetic\",\"num_rows\":32,\"package_num_channel\":0,\"ppg_channels\":[24,25],\"resistance_channels\":[27,28],\"sampling_rate\":250,\"temperature_channels\":[26],\"timestamp_channel\":30}
                    """
                    )
        
        XCTAssert(result == testBoard)
    }

    func testDeviceName () throws {
        let result = try BoardShim.getDeviceName(synthBoardID)
        XCTAssert(result == "Synthetic")
    }

    func testEEGchannels () throws {
        let result = try BoardShim.getEEGchannels(synthBoardID)
        XCTAssert(result == [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16])
    }

    func testEMGchannels () throws
    {
        let result = try BoardShim.getEMGchannels(synthBoardID)
        XCTAssert(result == [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16])
    }
    
    func testECGchannels () throws {
        let result = try BoardShim.getECGchannels(synthBoardID)
        XCTAssert(result == [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16])
    }

    func testTemperatureChannels () throws {
        let result = try BoardShim.getTemperatureChannels(synthBoardID)
        XCTAssert(result == [26])
    }

    func testResistanceChannels () throws {
        let result = try BoardShim.getResistanceChannels(synthBoardID)
        XCTAssert(result == [27,28])
    }

    func testEOGchannels () throws {
        let result = try BoardShim.getEOGchannels(synthBoardID)
        XCTAssert(result == [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16])
    }

    func testEXGchannels () throws {
        let result = try BoardShim.getEXGchannels(synthBoardID)
        XCTAssert(result == [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16])
    }

    func testEDAchannels () throws {
        let result = try BoardShim.getEXGchannels(synthBoardID)
        XCTAssert(result == [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16])
    }

    func testPPGchannels () throws {
        let result = try BoardShim.getEXGchannels(synthBoardID)
        XCTAssert(result == [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16])
    }

    func testBoardPresets () throws {
        let presets = try BoardShim.getBoardPresets(synthBoardID)
        XCTAssert(presets == [1,0])
    }

    func testAccelChannels () throws {
        let result = try BoardShim.getAccelChannels(synthBoardID, .DEFAULT_PRESET)
        XCTAssert(result == [17,18,19])
    }

    func testGyroChannels () throws {
        let result = try BoardShim.getGyroChannels(synthBoardID)
        XCTAssert(result == [20,21,22])
    }

    func testAnalogChannels () throws {
        XCTAssertThrowsError(try BoardShim.getAnalogChannels(synthBoardID), "test message") { error in
            XCTAssertEqual(error as? BrainFlowException, BrainFlowException("Error in board info getter", .UNSUPPORTED_BOARD_ERROR))
          }
    }

    func testOtherChannels () throws {
        XCTAssertThrowsError(try BoardShim.getOtherChannels(synthBoardID), "test message") { error in
            XCTAssertEqual(error as? BrainFlowException, BrainFlowException("Error in board info getter", .UNSUPPORTED_BOARD_ERROR))
          }
    }

    func testMasterBoardID () throws {
        let result = try BoardShim.getMasterBoardID(boardId: synthBoardID, params: params)
        XCTAssert(result == .SYNTHETIC_BOARD)
    }

    func testConfigBoard () throws {
        let params = BrainFlowInputParams()
        let board = try BoardShim(synthBoardID, params)
        try board.prepareSession()
        do {
            let result = try board.configBoard("set_index_percentage:10")
            XCTAssert(result == "Config:set_index_percentage:10")
        } catch let bfError as BrainFlowException {
                try? BoardShim.logMessage (.LEVEL_ERROR, bfError.message)
                try? BoardShim.logMessage (.LEVEL_ERROR, "Error code: \(bfError.errorCode)")
                XCTFail()
        }
    }

    func testPerformanceExample() throws {
        // This is an example of a performance test case.
        self.measure {
            // Put the code you want to measure the time of here.
        }
    }
}


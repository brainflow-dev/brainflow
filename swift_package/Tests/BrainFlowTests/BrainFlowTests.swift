import XCTest
@testable import BrainFlow

final class BrainFlowTests: XCTestCase {
    private func requireNativeLibraries() throws {
        do {
            _ = try BoardShim.get_version()
        } catch {
            throw XCTSkip("BrainFlow native libraries are not available; build BrainFlow into installed/lib or set BRAINFLOW_LIB_DIR.")
        }
    }

    func testInputParamsJSON() throws {
        var params = BrainFlowInputParams()
        params.serial_port = "/dev/ttyUSB0"
        params.set_master_board(.SYNTHETIC_BOARD)
        let json = try params.to_json()

        XCTAssertTrue(json.contains("serial_port"))
        XCTAssertTrue(json.contains("ttyUSB0"))
        XCTAssertTrue(json.contains("master_board"))
    }

    func testBrainFlowGetDataSyntheticBoard() throws {
        try requireNativeLibraries()
        let board = try BoardShim(board_id: .SYNTHETIC_BOARD)
        try board.prepare_session()
        XCTAssertTrue(try board.is_prepared())

        try board.start_stream(buffer_size: 45000)
        Thread.sleep(forTimeInterval: 1.0)
        XCTAssertGreaterThan(try board.get_board_data_count(), 0)

        let currentData = try board.get_current_board_data(num_samples: 16)
        XCTAssertEqual(currentData.count, try BoardShim.get_num_rows(board_id: BoardIds.SYNTHETIC_BOARD.rawValue))
        XCTAssertLessThanOrEqual(currentData.first?.count ?? 0, 16)

        try board.stop_stream()
        let data = try board.get_board_data()
        try board.release_session()

        XCTAssertEqual(data.count, try BoardShim.get_num_rows(board_id: BoardIds.SYNTHETIC_BOARD.rawValue))
        XCTAssertGreaterThan(data.first?.count ?? 0, 0)
    }

    func testMarkersSyntheticBoard() throws {
        try requireNativeLibraries()
        let board = try BoardShim(board_id: .SYNTHETIC_BOARD)
        try board.prepare_session()
        try board.start_stream(buffer_size: 45000)
        try board.insert_marker(1.0)
        Thread.sleep(forTimeInterval: 0.5)
        try board.stop_stream()
        let data = try board.get_board_data()
        try board.release_session()

        let markerChannel = try BoardShim.get_marker_channel(board_id: BoardIds.SYNTHETIC_BOARD.rawValue)
        XCTAssertTrue(data[markerChannel].contains { abs($0 - 1.0) < 0.0001 })
    }

    func testReadWriteFile() throws {
        try requireNativeLibraries()
        let data = [
            [1.0, 2.0, 3.0],
            [4.0, 5.0, 6.0]
        ]
        let fileName = NSTemporaryDirectory() + "/brainflow_swift_read_write.csv"
        try DataFilter.write_file(data: data, file_name: fileName, file_mode: "w")
        let restored = try DataFilter.read_file(fileName)

        XCTAssertEqual(restored.count, data.count)
        XCTAssertEqual(restored.first?.count, data.first?.count)
    }

    func testDownsamplingAndTransforms() throws {
        try requireNativeLibraries()
        let data = Array(0..<128).map(Double.init)
        let downsampled = try DataFilter.perform_downsampling(data: data, period: 4, operation: .MEAN)
        XCTAssertEqual(downsampled.count, 32)

        let fft = try DataFilter.perform_fft(data: data, start_pos: 0, end_pos: 128, window: .NO_WINDOW)
        XCTAssertEqual(fft.count, 65)

        let restored = try DataFilter.perform_ifft(data: fft)
        XCTAssertEqual(restored.count, 128)
    }

    func testSignalFilteringDenoisingAndBandPower() throws {
        try requireNativeLibraries()
        var data = (0..<256).map { index in sin(Double(index) / 10.0) }
        try DataFilter.perform_lowpass(data: &data, sampling_rate: 250, cutoff: 30.0, order: 4, filter_type: .BUTTERWORTH, ripple: 0.0)
        try DataFilter.perform_highpass(data: &data, sampling_rate: 250, cutoff: 1.0, order: 4, filter_type: .BUTTERWORTH, ripple: 0.0)
        try DataFilter.perform_wavelet_denoising(
            data: &data,
            wavelet: .DB5,
            decomposition_level: 3,
            wavelet_denoising: .SURESHRINK,
            threshold: .HARD,
            extension_type: .SYMMETRIC,
            noise_level: .FIRST_LEVEL
        )

        let psd = try DataFilter.get_psd(data: data, start_pos: 0, end_pos: data.count, sampling_rate: 250, window: WindowOperations.HANNING.rawValue)
        let power = try DataFilter.get_band_power(psd: psd, freq_start: 4.0, freq_end: 30.0)
        XCTAssertTrue(power.isFinite)
    }

    func testICA() throws {
        try requireNativeLibraries()
        let rows = 4
        let cols = 128
        let data = (0..<rows).map { row in
            (0..<cols).map { col in sin(Double(col + row) / 8.0) + Double(row) }
        }
        let ica = try DataFilter.perform_ica(data: data, num_components: 2)

        XCTAssertEqual(ica.w.count, 2)
        XCTAssertEqual(ica.k.count, 2)
        XCTAssertEqual(ica.a.count, rows)
        XCTAssertEqual(ica.s.count, 2)
    }

    func testMLModelPrediction() throws {
        try requireNativeLibraries()
        let params = BrainFlowModelParams(metric: .MINDFULNESS, classifier: .DEFAULT_CLASSIFIER)
        let model = try MLModel(params: params)

        try model.prepare()
        let prediction = try model.predict(input_data: [0.2, 0.3, 0.4, 0.5, 0.6])
        try model.release()

        XCTAssertFalse(prediction.isEmpty)
        XCTAssertTrue(prediction.allSatisfy(\.isFinite))
    }
}

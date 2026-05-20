import BrainFlow
import Foundation
import SwiftUI

@main
struct BrainFlowiOSDemoApp: App {
    var body: some Scene {
        WindowGroup {
            ContentView()
        }
    }
}

private struct BoardOption: Identifiable {
    let id: Int
    let title: String
    let boardId: Int
}

private let boardOptions = [
    BoardOption(id: BoardIds.SYNTHETIC_BOARD.rawValue, title: "Synthetic", boardId: BoardIds.SYNTHETIC_BOARD.rawValue),
    BoardOption(id: BoardIds.MUSE_2_BOARD.rawValue, title: "Muse 2", boardId: BoardIds.MUSE_2_BOARD.rawValue),
    BoardOption(id: BoardIds.MUSE_S_BOARD.rawValue, title: "Muse S", boardId: BoardIds.MUSE_S_BOARD.rawValue),
    BoardOption(id: BoardIds.MUSE_2016_BOARD.rawValue, title: "Muse 2016", boardId: BoardIds.MUSE_2016_BOARD.rawValue),
    BoardOption(id: BoardIds.MUSE_S_ATHENA_BOARD.rawValue, title: "Muse S Athena", boardId: BoardIds.MUSE_S_ATHENA_BOARD.rawValue)
]

struct ContentView: View {
    @State private var selectedBoardId = BoardIds.SYNTHETIC_BOARD.rawValue
    @State private var activeBoardId = BoardIds.SYNTHETIC_BOARD.rawValue
    @State private var serialNumber = ""
    @State private var macAddress = ""
    @State private var timeout = "15"
    @State private var status = "Idle"
    @State private var sampleCount = 0
    @State private var rowCount = 0
    @State private var board: BoardShim?
    @State private var isStreaming = false
    @State private var didRunAutomatedDemo = false
    @State private var eegSeries = [[Double]]()

    var body: some View {
        NavigationView {
            Form {
                Section("Board") {
                    Picker("Board", selection: $selectedBoardId) {
                        ForEach(boardOptions) { option in
                            Text(option.title).tag(option.boardId)
                        }
                    }
                    .disabled(isStreaming)

                    if selectedBoardId != BoardIds.SYNTHETIC_BOARD.rawValue {
                        TextField("Serial Number", text: $serialNumber)
                            .textInputAutocapitalization(.never)
                            .disableAutocorrection(true)
                            .disabled(isStreaming)
                        TextField("MAC Address", text: $macAddress)
                            .textInputAutocapitalization(.never)
                            .disableAutocorrection(true)
                            .disabled(isStreaming)
                        TextField("Timeout", text: $timeout)
                            .keyboardType(.numberPad)
                            .disabled(isStreaming)
                    }
                }

                Section("Session") {
                    infoRow("Status", status)
                    infoRow("Rows", "\(rowCount)")
                    infoRow("Samples", "\(sampleCount)")
                }

                Section("EEG") {
                    EEGPlotView(series: eegSeries)
                        .frame(height: 160)
                        .padding(.vertical, 8)
                }

                Section {
                    Button(isStreaming ? "Stop Stream" : "Start Stream") {
                        isStreaming ? stopStream() : startStream()
                    }
                    Button("Read Data") {
                        readData()
                    }
                    .disabled(isStreaming || board == nil)
                    Button("Release Session") {
                        releaseSession()
                    }
                    .disabled(board == nil)
                }
            }
            .navigationTitle("BrainFlow Demo")
        }
        .navigationViewStyle(.stack)
        .task {
            await runAutomatedDemoIfRequested()
        }
    }

    private func infoRow(_ title: String, _ value: String) -> some View {
        HStack {
            Text(title)
            Spacer()
            Text(value)
                .foregroundColor(.secondary)
                .multilineTextAlignment(.trailing)
        }
    }

    private func startStream() {
        do {
            var params = BrainFlowInputParams()
            params.serial_number = serialNumber.trimmingCharacters(in: .whitespacesAndNewlines)
            params.mac_address = macAddress.trimmingCharacters(in: .whitespacesAndNewlines)
            params.timeout = Int(timeout) ?? 15

            let board = try BoardShim(board_id: selectedBoardId, input_params: params)
            try board.prepare_session()
            try board.start_stream(buffer_size: 45000)
            self.board = board
            activeBoardId = selectedBoardId
            status = "Streaming \(boardName(for: selectedBoardId))"
            isStreaming = true
        } catch {
            status = "Start failed: \(error)"
        }
    }

    private func stopStream() {
        do {
            try board?.stop_stream()
            status = "Stopped"
            isStreaming = false
        } catch {
            status = "Stop failed: \(error)"
        }
    }

    private func readData() {
        guard let board else {
            status = "No active session"
            return
        }

        do {
            let data = try board.get_board_data()
            updateDisplay(with: data, boardId: activeBoardId)
            status = "Read complete"
        } catch {
            status = "Read failed: \(error)"
        }
    }

    private func releaseSession() {
        do {
            try board?.release_session()
            board = nil
            isStreaming = false
            status = "Released"
        } catch {
            status = "Release failed: \(error)"
        }
    }

    private func updateDisplay(with data: [[Double]], boardId: Int) {
        rowCount = data.count
        sampleCount = data.first?.count ?? 0

        let eegChannels = (try? BoardShim.get_eeg_channels(board_id: boardId)) ?? []
        eegSeries = eegChannels.prefix(4).compactMap { channel in
            guard channel >= 0, channel < data.count else { return nil }
            return Array(data[channel].suffix(250))
        }
    }

    private func boardName(for boardId: Int) -> String {
        boardOptions.first { $0.boardId == boardId }?.title ?? "Board \(boardId)"
    }

    private func runAutomatedDemoIfRequested() async {
        let processInfo = ProcessInfo.processInfo
        let shouldAutorun = processInfo.environment["BRAINFLOW_IOS_DEMO_AUTORUN"] == "1" ||
            processInfo.arguments.contains("--autorun")
        guard !didRunAutomatedDemo, shouldAutorun else {
            return
        }

        didRunAutomatedDemo = true
        selectedBoardId = BoardIds.SYNTHETIC_BOARD.rawValue
        status = "Autorun starting"
        startStream()

        try? await Task.sleep(nanoseconds: 2_000_000_000)

        stopStream()
        readData()
        let rows = rowCount
        let samples = sampleCount
        releaseSession()

        if rows > 0 && samples > 0 {
            status = "Autorun passed: \(samples) samples"
            print("BrainFlowiOSDemo autorun passed rows=\(rows) samples=\(samples)")
        } else {
            status = "Autorun failed"
            print("BrainFlowiOSDemo autorun failed rows=\(rows) samples=\(samples)")
        }
    }
}

private struct EEGPlotView: View {
    let series: [[Double]]
    private let colors: [Color] = [.blue, .green, .orange, .purple]

    var body: some View {
        GeometryReader { proxy in
            ZStack {
                RoundedRectangle(cornerRadius: 8)
                    .fill(Color(.secondarySystemGroupedBackground))
                ForEach(Array(series.prefix(4).enumerated()), id: \.offset) { index, values in
                    path(for: values, channelIndex: index, channelCount: max(series.prefix(4).count, 1), size: proxy.size)
                        .stroke(colors[index % colors.count], lineWidth: 1.5)
                }
            }
        }
    }

    private func path(for values: [Double], channelIndex: Int, channelCount: Int, size: CGSize) -> Path {
        let samples = values.filter { $0.isFinite }
        guard samples.count > 1 else { return Path() }

        let minValue = samples.min() ?? 0.0
        let maxValue = samples.max() ?? 0.0
        let span = max(maxValue - minValue, 1.0)
        let laneHeight = size.height / CGFloat(channelCount)
        let laneTop = laneHeight * CGFloat(channelIndex)
        let lanePadding = laneHeight * 0.12
        let drawableHeight = max(laneHeight - lanePadding * 2, 1)
        let stepX = size.width / CGFloat(samples.count - 1)

        var path = Path()
        for (index, sample) in samples.enumerated() {
            let normalized = (sample - minValue) / span
            let x = CGFloat(index) * stepX
            let y = laneTop + lanePadding + CGFloat(1.0 - normalized) * drawableHeight
            if index == 0 {
                path.move(to: CGPoint(x: x, y: y))
            } else {
                path.addLine(to: CGPoint(x: x, y: y))
            }
        }
        return path
    }
}

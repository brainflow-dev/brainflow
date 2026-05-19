import BrainFlow
import SwiftUI

@main
struct BrainFlowiOSDemoApp: App {
    var body: some Scene {
        WindowGroup {
            ContentView()
        }
    }
}

struct ContentView: View {
    @State private var status = "Idle"
    @State private var sampleCount = 0
    @State private var rowCount = 0
    @State private var board: BoardShim?
    @State private var isStreaming = false
    @State private var didRunAutomatedDemo = false

    var body: some View {
        NavigationView {
            Form {
                Section("Synthetic Board") {
                    infoRow("Status", status)
                    infoRow("Rows", "\(rowCount)")
                    infoRow("Samples", "\(sampleCount)")
                }

                Section {
                    Button(isStreaming ? "Stop Stream" : "Start Stream") {
                        isStreaming ? stopStream() : startStream()
                    }
                    Button("Read Data") {
                        readData()
                    }
                    .disabled(isStreaming)
                    Button("Release Session") {
                        releaseSession()
                    }
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
            let board = try BoardShim(board_id: BoardIds.SYNTHETIC_BOARD)
            try board.prepare_session()
            try board.start_stream(buffer_size: 45000)
            self.board = board
            status = "Streaming"
            isStreaming = true
        } catch {
            status = "Start failed"
        }
    }

    private func stopStream() {
        do {
            try board?.stop_stream()
            status = "Stopped"
            isStreaming = false
        } catch {
            status = "Stop failed"
        }
    }

    private func readData() {
        do {
            let data = try board?.get_board_data() ?? []
            rowCount = data.count
            sampleCount = data.first?.count ?? 0
            status = "Read complete"
        } catch {
            status = "Read failed"
        }
    }

    private func releaseSession() {
        do {
            try board?.release_session()
            board = nil
            isStreaming = false
            status = "Released"
        } catch {
            status = "Release failed"
        }
    }

    private func runAutomatedDemoIfRequested() async {
        let processInfo = ProcessInfo.processInfo
        let shouldAutorun = processInfo.environment["BRAINFLOW_IOS_DEMO_AUTORUN"] == "1" ||
            processInfo.arguments.contains("--autorun")
        guard !didRunAutomatedDemo, shouldAutorun else {
            return
        }

        didRunAutomatedDemo = true
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

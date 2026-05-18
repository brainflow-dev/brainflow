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

    var body: some View {
        NavigationStack {
            Form {
                Section("Synthetic Board") {
                    LabeledContent("Status", value: status)
                    LabeledContent("Rows", value: "\(rowCount)")
                    LabeledContent("Samples", value: "\(sampleCount)")
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
}

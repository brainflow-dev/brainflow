import BrainFlow
import SwiftUI

#if os(macOS)
import AppKit
#endif

@main
struct BrainFlowMacDemoApp: App {
    private let autorun = ProcessInfo.processInfo.environment["BRAINFLOW_MAC_DEMO_AUTORUN"] == "1"

    var body: some Scene {
        WindowGroup {
            ContentView(autorun: autorun)
        }
    }
}

struct ContentView: View {
    let autorun: Bool

    @State private var status = "Idle"
    @State private var rows = 0
    @State private var cols = 0
    @State private var isRunning = false
    @State private var board: BoardShim?
    @State private var didAutorun = false

    var body: some View {
        VStack(alignment: .leading, spacing: 16) {
            Text("BrainFlow Synthetic Board")
                .font(.title2)
                .fontWeight(.semibold)

            VStack(alignment: .leading, spacing: 8) {
                infoRow("Status", status)
                infoRow("Rows", "\(rows)")
                infoRow("Samples", "\(cols)")
            }

            HStack {
                Button(isRunning ? "Stop" : "Start") {
                    isRunning ? stop() : start()
                }
                .keyboardShortcut(.defaultAction)

                Button("Read") {
                    read()
                }
                .disabled(isRunning)

                Button("Release") {
                    release()
                }
            }
        }
        .padding(24)
        .frame(minWidth: 420, minHeight: 240)
        .task {
            guard autorun, !didAutorun else { return }
            didAutorun = true
            await runAutomatedDemo()
        }
    }

    private func infoRow(_ label: String, _ value: String) -> some View {
        HStack(alignment: .firstTextBaseline) {
            Text(label)
                .fontWeight(.medium)
                .frame(width: 84, alignment: .leading)
            Text(value)
                .frame(maxWidth: .infinity, alignment: .leading)
        }
    }

    private func start() {
        do {
            let board = try BoardShim(board_id: BoardIds.SYNTHETIC_BOARD)
            try board.prepare_session()
            try board.start_stream(buffer_size: 45000)
            self.board = board
            status = "Streaming synthetic data"
            isRunning = true
        } catch {
            status = "Start failed: \(error)"
        }
    }

    private func stop() {
        do {
            try board?.stop_stream()
            isRunning = false
            status = "Stopped"
        } catch {
            status = "Stop failed: \(error)"
        }
    }

    private func read() {
        do {
            let data = try board?.get_board_data() ?? []
            rows = data.count
            cols = data.first?.count ?? 0
            status = "Read \(cols) samples"
        } catch {
            status = "Read failed: \(error)"
        }
    }

    private func release() {
        do {
            try board?.release_session()
            board = nil
            isRunning = false
            status = "Released"
        } catch {
            status = "Release failed: \(error)"
        }
    }

    @MainActor
    private func runAutomatedDemo() async {
        start()
        guard isRunning else {
            print("BrainFlowMacDemo virtual board demo failed: \(status)")
            terminateIfRequested()
            return
        }

        try? await Task.sleep(nanoseconds: 2_000_000_000)
        stop()
        read()
        let measuredRows = rows
        let measuredCols = cols
        release()
        status = "Demo complete: \(measuredCols) samples"
        print("BrainFlowMacDemo virtual board demo passed: rows=\(measuredRows) samples=\(measuredCols)")
        terminateIfRequested()
    }

    private func terminateIfRequested() {
        guard ProcessInfo.processInfo.environment["BRAINFLOW_MAC_DEMO_EXIT_AFTER_AUTORUN"] == "1" else { return }
        #if os(macOS)
        NSApplication.shared.terminate(nil)
        #endif
    }
}

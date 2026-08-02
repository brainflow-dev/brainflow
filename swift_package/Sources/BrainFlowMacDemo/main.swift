import BrainFlow
import Foundation
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
    @State private var pollingTask: Task<Void, Never>?
    @State private var eegSeries = [[Double]]()

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

            EEGPlotView(series: eegSeries)
                .frame(height: 150)

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
        .frame(minWidth: 520, minHeight: 420)
        .task {
            guard autorun, !didAutorun else { return }
            didAutorun = true
            await runAutomatedDemo()
        }
        .onDisappear {
            pollingTask?.cancel()
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
            pollingTask?.cancel()
            try? board?.release_session()

            let board = try BoardShim(board_id: BoardIds.SYNTHETIC_BOARD)
            try board.prepare_session()
            try board.start_stream(buffer_size: 45000)
            self.board = board
            rows = try BoardShim.get_num_rows(board_id: BoardIds.SYNTHETIC_BOARD)
            cols = 0
            eegSeries = []
            status = "Streaming synthetic data"
            isRunning = true
            startPolling()
        } catch {
            status = "Start failed: \(error)"
        }
    }

    private func stop() {
        do {
            pollingTask?.cancel()
            pollCurrentData()
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
            updateDisplay(with: data)
            status = "Read \(cols) samples"
        } catch {
            status = "Read failed: \(error)"
        }
    }

    private func release() {
        do {
            pollingTask?.cancel()
            if isRunning {
                try board?.stop_stream()
            }
            try board?.release_session()
            board = nil
            isRunning = false
            status = "Released"
        } catch {
            status = "Release failed: \(error)"
        }
    }

    private func startPolling() {
        pollingTask?.cancel()
        pollingTask = Task { @MainActor in
            while !Task.isCancelled {
                pollCurrentData()
                try? await Task.sleep(nanoseconds: 250_000_000)
            }
        }
    }

    private func pollCurrentData() {
        guard let board else { return }

        do {
            let bufferedSamples = try board.get_board_data_count()
            let previewSamples = max(min(bufferedSamples, 250), 1)
            let data = try board.get_current_board_data(num_samples: previewSamples)
            updateDisplay(with: data, sampleCount: bufferedSamples)
        } catch {
            status = "Read failed: \(error)"
        }
    }

    private func updateDisplay(with data: [[Double]], sampleCount: Int? = nil) {
        rows = data.count
        cols = sampleCount ?? (data.first?.count ?? 0)

        let eegChannels = (try? BoardShim.get_eeg_channels(board_id: BoardIds.SYNTHETIC_BOARD)) ?? []
        eegSeries = eegChannels.prefix(4).compactMap { channel in
            guard channel >= 0, channel < data.count else { return nil }
            return Array(data[channel].suffix(250))
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

private struct EEGPlotView: View {
    let series: [[Double]]
    private let colors: [Color] = [.blue, .green, .orange, .purple]

    var body: some View {
        GeometryReader { proxy in
            ZStack(alignment: .topLeading) {
                RoundedRectangle(cornerRadius: 8)
                    .fill(Color.secondary.opacity(0.12))

                ForEach(Array(series.prefix(4).enumerated()), id: \.offset) { index, values in
                    path(for: values, channelIndex: index, channelCount: max(series.prefix(4).count, 1), size: proxy.size)
                        .stroke(colors[index % colors.count], lineWidth: 1.5)
                }

                if series.isEmpty {
                    Text("Waiting for samples")
                        .foregroundStyle(.secondary)
                        .padding(12)
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

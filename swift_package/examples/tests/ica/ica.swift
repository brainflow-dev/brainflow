import BrainFlow
import BrainFlowExampleSupport

@main
enum ICAExample {
    static func main() throws {
        let sample = try SyntheticBoardDataReader.read(maxSamples: 500)
        let channels = Array(sample.eegChannels.prefix(4))
        let ica = try DataFilter.perform_ica(data: sample.data, num_components: 2, channels: channels)

        print("W: \(ica.w.count)x\(ica.w.first?.count ?? 0)")
        print("S: \(ica.s.count)x\(ica.s.first?.count ?? 0)")
    }
}

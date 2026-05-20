import BrainFlow
import BrainFlowExampleSupport
import Foundation

@main
enum ReadWriteFileExample {
    static func main() throws {
        let sample = try SyntheticBoardDataReader.read(seconds: 2.0)

        let fileName = NSTemporaryDirectory() + "/brainflow_swift.csv"
        try DataFilter.write_file(data: sample.data, file_name: fileName, file_mode: "w")
        let restored = try DataFilter.read_file(fileName)
        print("Original: \(sample.data.count)x\(sample.data.first?.count ?? 0)")
        print("Restored: \(restored.count)x\(restored.first?.count ?? 0)")
    }
}

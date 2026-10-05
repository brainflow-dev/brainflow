import Foundation
import BrainFlow

@main
enum ICAExample {
    static func main() throws {
        // Two simultaneous mixtures: rows are channels and columns are samples.
        let samples = 1024
        var data = [[Double]](repeating: [Double](repeating: 0, count: samples), count: 2)
        for i in 0..<samples {
            let t = Double(i) / 256.0
            let first = sin(2.0 * Double.pi * 7.0 * t)
            let second = pow(sin(2.0 * Double.pi * 13.0 * t), 3.0)
            data[0][i] = first + 0.3 * second
            data[1][i] = 0.2 * first + second
        }
        let ica = try DataFilter.perform_ica(data: data, num_components: 2)
        // Component order and sign are arbitrary.
        print("Recovered \(ica.s.count) sources from \(samples) samples")
    }
}

import BrainFlow

let data = Array(0..<256).map(Double.init)
let downsampled = try DataFilter.perform_downsampling(data: data, period: 4, operation: AggOperations.MEAN)
print(downsampled)

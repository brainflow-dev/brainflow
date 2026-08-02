import BrainFlow
import BrainFlowExampleSupport

@main
enum EEGMetricsExample {
    static func main() throws {
        let sample = try SyntheticBoardDataReader.read()
        let bandPowers = try DataFilter.get_avg_band_powers(
            data: sample.data,
            channels: sample.eegChannels,
            sampling_rate: sample.samplingRate,
            apply_filter: true
        )

        let params = BrainFlowModelParams(metric: BrainFlowMetrics.MINDFULNESS, classifier: BrainFlowClassifiers.DEFAULT_CLASSIFIER)
        let model = try MLModel(params: params)
        try model.prepare()
        let prediction = try model.predict(input_data: bandPowers.average)
        try model.release()
        print(prediction)
    }
}

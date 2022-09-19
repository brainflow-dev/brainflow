use std::{thread, time::Duration};

use brainflow::{
    board_shim, brainflow_input_params::BrainFlowInputParamsBuilder,
    brainflow_model_params::BrainFlowModelParamsBuilder, data_filter, ml_model, BoardIds,
    BrainFlowClassifiers, BrainFlowMetrics, BrainFlowPresets,
};

fn main() {
    brainflow::board_shim::enable_dev_board_logger().unwrap();
    let params = BrainFlowInputParamsBuilder::default().build();
    let board_id = BoardIds::SyntheticBoard;
    let board = board_shim::BoardShim::new(board_id, params).unwrap();

    board.prepare_session().unwrap();
    board.start_stream(45000, "").unwrap();
    thread::sleep(Duration::from_secs(5));
    board.stop_stream().unwrap();
    let data = board.get_board_data(None, BrainFlowPresets::DefaultPreset).unwrap();
    board.release_session().unwrap();

    let eeg_channels = board_shim::get_eeg_channels(board_id, BrainFlowPresets::DefaultPreset).unwrap();
    let sampling_rate = board_shim::get_sampling_rate(board_id, BrainFlowPresets::DefaultPreset).unwrap();
    let bands =
        data_filter::get_avg_band_powers(data, eeg_channels, sampling_rate, true).unwrap();
    let mut feature_vector = bands.0;
    println!("feature_vector: {:?}", feature_vector);

    let model_params = BrainFlowModelParamsBuilder::new()
        .metric(BrainFlowMetrics::Mindfulness)
        .classifier(BrainFlowClassifiers::DefaultClassifier)
        .build();
    let mindfulness = ml_model::MlModel::new(model_params).unwrap();
    mindfulness.prepare().unwrap();
    println!(
        "Mindfulness: {:?}",
        mindfulness.predict(&mut feature_vector)
    );
    mindfulness.release().unwrap();
}

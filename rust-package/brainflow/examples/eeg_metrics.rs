use std::{thread, time::Duration};

use brainflow::{
    board_shim, brainflow_input_params::BrainFlowInputParamsBuilder,
    brainflow_model_params::BrainFlowModelParamsBuilder, data_filter, ml_model, BoardIds,
    BrainFlowClassifiers, BrainFlowMetrics,
};

fn main() {
    brainflow::board_shim::enable_dev_board_logger().unwrap();
    let params = BrainFlowInputParamsBuilder::default().build();
    let board_id = BoardIds::SyntheticBoard as i32;
    let board = board_shim::BoardShim::new(board_id, params).unwrap();

    board.prepare_session().unwrap();
    board.start_stream(45000, "").unwrap();
    thread::sleep(Duration::from_secs(5));
    board.stop_stream().unwrap();
    let data = board.get_board_data(None).unwrap();
    board.release_session().unwrap();

    let eeg_channels = board_shim::get_eeg_channels(board_id).unwrap();
    let sampling_rate = board_shim::get_sampling_rate(board_id).unwrap();
    let mut bands =
        data_filter::get_avg_band_powers(data, eeg_channels, sampling_rate, true).unwrap();
    let mut feature_vector = bands.0;
    feature_vector.append(&mut bands.1);
    println!("feature_vector: {:?}", feature_vector);

    // calc concentration
    let concentration_params = BrainFlowModelParamsBuilder::new()
        .metric(BrainFlowMetrics::Concentration)
        .classifier(BrainFlowClassifiers::Knn)
        .build();
    let concentration = ml_model::MlModel::new(concentration_params).unwrap();
    concentration.prepare().unwrap();
    println!(
        "Concentration: {:?}",
        concentration.predict(&mut feature_vector)
    );
    concentration.release().unwrap();

    // calc relaxation
    let relaxation_params = BrainFlowModelParamsBuilder::new()
        .metric(BrainFlowMetrics::Relaxation)
        .classifier(BrainFlowClassifiers::Regression)
        .build();
    let relaxation = ml_model::MlModel::new(relaxation_params).unwrap();
    relaxation.prepare().unwrap();
    println!("Relaxation: {:?}", relaxation.predict(&mut feature_vector));
    relaxation.release().unwrap();
}

use std::{thread, time::Duration};

use brainflow::{board_shim, brainflow_input_params::BrainFlowInputParamsBuilder, BoardIds};

pub fn board_data(n_data_points: Option<usize>) -> Vec<Vec<f64>> {
    brainflow::board_shim::enable_dev_board_logger().unwrap();
    let params = BrainFlowInputParamsBuilder::default().build();
    let board_id = BoardIds::SyntheticBoard as i32;
    let board = board_shim::BoardShim::new(board_id, params).unwrap();

    board.prepare_session().unwrap();
    board.start_stream(45000, "").unwrap();
    thread::sleep(Duration::from_secs(5));
    board.stop_stream().unwrap();
    let data = board.get_board_data(n_data_points).unwrap();
    board.release_session().unwrap();

    data
}

use std::{thread, time::Duration};

use brainflow::{board_shim, brainflow_input_params::BrainFlowInputParamsBuilder, BoardIds};

fn main() {
    brainflow::board_shim::enable_dev_board_logger().unwrap();
    let params = BrainFlowInputParamsBuilder::default().build();
    let board = board_shim::BoardShim::new(BoardIds::SyntheticBoard, params).unwrap();

    board.prepare_session().unwrap();
    board.start_stream(45000, "").unwrap();
    thread::sleep(Duration::from_secs(5));
    board.stop_stream().unwrap();
    let data = board.get_board_data(Some(10)).unwrap();
    board.release_session().unwrap();

    println!("{}", data.len());
    println!("{:?}", data);
}

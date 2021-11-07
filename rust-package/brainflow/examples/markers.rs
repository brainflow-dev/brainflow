use std::thread;
use std::time::Duration;

use brainflow::board_shim::BoardShim;
use brainflow::brainflow_input_params::BrainFlowInputParamsBuilder;
use brainflow::BoardIds;

fn main() {
    brainflow::board_shim::enable_dev_board_logger().unwrap();
    let params = BrainFlowInputParamsBuilder::default().build();
    let board = BoardShim::new(BoardIds::SyntheticBoard, params).unwrap();

    board.prepare_session().unwrap();
    board.start_stream(45000, "file://data.csv:w").unwrap();
    thread::sleep(Duration::from_secs(5));
    board.insert_marker(1.0).unwrap();
    thread::sleep(Duration::from_secs(5));
    board.stop_stream().unwrap();
    let data = board.get_board_data(Some(10)).unwrap();
    board.release_session().unwrap();

    println!("{:?}", data);
}

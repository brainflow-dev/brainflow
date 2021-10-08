use std::thread;
use std::time::Duration;

use brainflow::board_shim::BoardShim;
use brainflow::brainflow_input_params::BrainFlowInputParamsBuilder;
use brainflow::BoardId;

fn main() {
    brainflow::board_shim::enable_dev_board_logger().unwrap();
    brainflow::ml_model::enable_ml_logger().unwrap();
    let params = BrainFlowInputParamsBuilder::default()
        .serial_port("/dev/ttyUSB0")
        .build();
    let board = BoardShim::new(BoardId::CytonBoard, params).unwrap();

    dbg!("board instantiated");

    board.prepare_session().unwrap();

    board.start_stream(20, "").unwrap();
    thread::sleep(Duration::from_secs(5));
    board.stop_stream().unwrap();

    let cnt = dbg!(board.get_board_data_count().unwrap());
    let data = board.get_current_board_data(cnt).unwrap();
    board.release_session().unwrap();

    dbg!(data);
}

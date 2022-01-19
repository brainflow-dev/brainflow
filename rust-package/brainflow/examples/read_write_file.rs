use std::{env, fs, thread, time::Duration};

use brainflow::{
    board_shim, brainflow_input_params::BrainFlowInputParamsBuilder, data_filter, BoardIds,
};

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

    let mut tmp_dir = env::temp_dir();
    tmp_dir.push("brainflow_tests");
    tmp_dir.push("rust");
    fs::create_dir_all(&tmp_dir).unwrap();
    tmp_dir.push("read-write_file.csv");

    let filename = tmp_dir.to_str().unwrap();

    dbg!(&data);
    data_filter::write_file(&data, filename, "w").unwrap();
    let read_data = data_filter::read_file(filename).unwrap();
    dbg!(read_data);
}

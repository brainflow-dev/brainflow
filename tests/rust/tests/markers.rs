use std::thread;
use std::time::Duration;

use brainflow::board_shim::BoardShim;
use brainflow::brainflow_input_params::BrainFlowInputParamsBuilder;
use brainflow::BoardIds;

#[test]
fn markers() {
    brainflow::board_shim::enable_dev_board_logger().unwrap();
    let params = BrainFlowInputParamsBuilder::default().build();
    let board = BoardShim::new(BoardIds::SyntheticBoard as i32, params).unwrap();

    board.prepare_session().unwrap();
    board.start_stream(45000, "file://data.csv:w").unwrap();
    thread::sleep(Duration::from_secs(5));
    board.insert_marker(1.0).unwrap();
    thread::sleep(Duration::from_secs(5));
    board.stop_stream().unwrap();
    let data = board.get_board_data(Some(10)).unwrap();
    board.release_session().unwrap();

    assert_eq!(data.len(), 32);
    println!("{:?}", data);

    // check that the first array in data are sequential numbers
    for (d, d_plus_1) in data[0].iter().zip(data[0].iter().skip(1)) {
        assert_eq!(d + 1.0, *d_plus_1);
    }
}

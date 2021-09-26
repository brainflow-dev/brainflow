use brainflow::board_shim::BoardShim;
use brainflow::brainflow_input_params::BrainFlowInputParamsBuilder;
use brainflow::BoardId;

fn main() {
    let params = BrainFlowInputParamsBuilder::default()
        .serial_port("/dev/ttyUSB0")
        .build();
    let board = BoardShim::new(BoardId::CytonBoard, params).unwrap();

    dbg!("board instantiated");

    board.prepare_session().unwrap();

    dbg!(brainflow::board_shim::eeg_names(BoardId::CytonBoard).unwrap());
    dbg!(brainflow::board_shim::eeg_channels(BoardId::CytonBoard).unwrap());

    dbg!(board.is_prepared().unwrap());
}

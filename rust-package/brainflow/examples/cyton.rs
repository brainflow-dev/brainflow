use brainflow::board_shim::BoardShim;
use brainflow::brainflow_input_params::BrainFlowInputParamsBuilder;
use brainflow::ml_model::{BrainFlowModelParamsBuilder, MlModel};
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

    dbg!(brainflow::board_shim::eeg_names(BoardId::CytonBoard).unwrap());
    dbg!(brainflow::board_shim::eeg_channels(BoardId::CytonBoard).unwrap());

    dbg!(board.is_prepared().unwrap());

    let ml_params = BrainFlowModelParamsBuilder::default().build();
    let ml = MlModel::new(ml_params).unwrap();

    ml.prepare().unwrap();
}

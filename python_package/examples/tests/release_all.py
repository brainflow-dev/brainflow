from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds
from brainflow.ml_model import MLModel, BrainFlowMetrics, BrainFlowClassifiers, BrainFlowModelParams


def main():
    BoardShim.enable_dev_board_logger()

    params1 = BrainFlowInputParams()
    params1.other_info = 'board1'
    board_id = BoardIds.SYNTHETIC_BOARD.value
    params2 = BrainFlowInputParams()
    params2.other_info = 'board2'

    board1 = BoardShim(board_id, params1)
    board1.prepare_session()
    board2 = BoardShim(board_id, params2)
    board2.prepare_session()

    BoardShim.release_all_sessions()

    ml_params1 = BrainFlowModelParams(BrainFlowMetrics.RESTFULNESS.value, BrainFlowClassifiers.DEFAULT_CLASSIFIER.value)
    model1 = MLModel(ml_params1)
    model1.prepare()
    ml_params2 = BrainFlowModelParams(BrainFlowMetrics.MINDFULNESS.value, BrainFlowClassifiers.DEFAULT_CLASSIFIER.value)
    model2 = MLModel(ml_params2)
    model2.prepare()

    MLModel.release_all()


if __name__ == "__main__":
    main()

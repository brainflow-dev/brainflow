import {
    BoardIds,
    BoardShim,
    BrainFlowClassifiers,
    BrainFlowMetrics,
    DataFilter,
    MLModel
} from 'brainflow';

function sleep (ms: number)
{
    return new Promise ((resolve) => { setTimeout (resolve, ms); });
}

async function runExample (): Promise<void>
{
    const boardId = BoardIds.SYNTHETIC_BOARD;
    const board = new BoardShim (boardId, {});
    board.prepareSession();
    board.startStream();
    await sleep (3000);
    board.stopStream();
    const data = board.getBoardData();
    board.releaseSession()
    console.info(data);
    const eegChannels = BoardShim.getEegChannels(boardId);
    const samplingRate = BoardShim.getSamplingRate(boardId);
    const bands = DataFilter.getAvgBandPowers(data, eegChannels, samplingRate, true);
    const model =
        new MLModel (BrainFlowMetrics.RESTFULNESS, BrainFlowClassifiers.DEFAULT_CLASSIFIER, {});
    model.prepare();
    console.info(model.predict(bands[0]));
    model.release();
}

runExample ();

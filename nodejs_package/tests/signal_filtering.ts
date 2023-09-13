import {BoardIds, BoardShim, DataFilter, FilterTypes} from 'brainflow';

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
    console.info('Data');
    console.info(data);
    const eegChannels = BoardShim.getEegChannels(boardId);
    const samplingRate = BoardShim.getSamplingRate(boardId);
    DataFilter.performBandPass(
        data[eegChannels[0]], samplingRate, 3.0, 20.0, 2, FilterTypes.BUTTERWORTH_ZERO_PHASE, 0.0);
    DataFilter.performBandStop(
        data[eegChannels[0]], samplingRate, 48.0, 52.0, 2, FilterTypes.BUTTERWORTH_ZERO_PHASE, 0.0);
    DataFilter.performLowPass(
        data[eegChannels[0]], samplingRate, 48.0, 2, FilterTypes.BUTTERWORTH_ZERO_PHASE, 0.0);
    DataFilter.performHighPass(
        data[eegChannels[0]], samplingRate, 3.0, 2, FilterTypes.BUTTERWORTH_ZERO_PHASE, 0.0);
}

runExample ();

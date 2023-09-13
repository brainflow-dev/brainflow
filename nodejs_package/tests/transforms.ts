import {
    BoardIds,
    BoardShim,
    DataFilter,
    WaveletExtensionTypes,
    WaveletTypes,
    WindowOperations
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
    const data = board.getCurrentBoardData(64);
    board.releaseSession()
    const eegChannels = BoardShim.getEegChannels(boardId);
    const oldData = data[eegChannels[0]];
    console.info(oldData);
    const fftData = DataFilter.performFft(oldData, WindowOperations.NO_WINDOW);
    console.info(fftData);
    const newData = DataFilter.performIfft(fftData);
    console.info(newData);
    const waveletData = DataFilter.performWaveletTransform(
        newData, WaveletTypes.DB2, 2, WaveletExtensionTypes.SYMMETRIC);
    const restoredData = DataFilter.performInverseWaveletTransform(
        waveletData, newData.length, WaveletTypes.DB2, 2, WaveletExtensionTypes.SYMMETRIC);
    console.info(restoredData);
}

runExample ();

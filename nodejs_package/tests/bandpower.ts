import {BoardIds, BoardShim, DataFilter, WindowOperations} from 'brainflow';

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
    await sleep (5000);
    board.stopStream();
    const data = board.getCurrentBoardData(128);
    board.releaseSession()
    const eegChannels = BoardShim.getEegChannels(boardId);
    const samplingRate = BoardShim.getSamplingRate(boardId);
    const oldData = data[eegChannels[1]];
    const psd = DataFilter.getPsdWelch(oldData, 64, 0.5, samplingRate, WindowOperations.HAMMING);
    const alpha = DataFilter.getBandPower(psd, 7.0, 13.0);
    const beta = DataFilter.getBandPower(psd, 14.0, 30.0);
    console.info(alpha / beta);
}

runExample ();

import {BoardIds, BoardShim, DataFilter, WaveletTypes} from 'brainflow';

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
    const data = board.getBoardData();
    board.releaseSession()
    const eegChannels = BoardShim.getEegChannels(boardId);
    const oldData = data[eegChannels[0]];
    console.info(oldData);
    DataFilter.performWaveletDenoising(oldData, WaveletTypes.DB2, 2);
    console.info(oldData);
}

runExample ();

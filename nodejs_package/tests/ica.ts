import {BoardIds, BoardShim, DataFilter} from 'brainflow';

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
    await sleep (10000);
    board.stopStream();
    const data = board.getCurrentBoardData(500);
    board.releaseSession()
    const eegChannels = BoardShim.getEegChannels(boardId);
    const eegData = data[eegChannels[0]];
    const eeg2D: number[][] = [];
    while (eegData.length)
        eeg2D.push(eegData.splice(0, 100));
    const icaData = DataFilter.performIca(eeg2D, 2, [0, 1, 2, 3, 4]);
    console.info(icaData[3]);
}

runExample ();

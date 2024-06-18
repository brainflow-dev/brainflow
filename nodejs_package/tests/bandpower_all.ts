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
    await sleep (4000);
    board.stopStream();
    const data = board.getBoardData();
    board.releaseSession()
    const eegChannels = BoardShim.getEegChannels(boardId);
    const samplingRate = BoardShim.getSamplingRate(boardId);
    const bandPowers =
        DataFilter.getAvgBandPowers(data, [eegChannels[0], eegChannels[1]], samplingRate)
    console.info(bandPowers);
}

runExample ();

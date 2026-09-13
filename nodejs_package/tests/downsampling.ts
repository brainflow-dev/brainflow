import {AggOperations, BoardIds, BoardShim, DataFilter} from 'brainflow';

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
    const data = board.getCurrentBoardData(10);
    board.releaseSession();
    const eegChannels = BoardShim.getEegChannels(boardId);

    // Re-reference every EEG channel using the sample-wise mean of the first two EEG channels.
    // The operation changes data in-place.
    if (eegChannels.length >= 2)
    {
        DataFilter.reference(data, eegChannels, eegChannels.slice(0, 2));
    }

    const oldData = data[eegChannels[0]];
    console.info(oldData);
    const newData = DataFilter.performDownsampling(oldData, 3, AggOperations.MEAN);
    console.info(newData);
}

runExample ();

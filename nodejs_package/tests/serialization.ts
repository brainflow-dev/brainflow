import {BoardIds, BoardShim, DataFilter} from 'brainflow';

function sleep (ms: number)
{
    return new Promise ((resolve) => { setTimeout (resolve, ms); });
}

async function runExample (): Promise<void>
{
    const board = new BoardShim (BoardIds.SYNTHETIC_BOARD, {});
    board.prepareSession();
    board.startStream();
    await sleep (3000);
    board.stopStream();
    const data = board.getCurrentBoardData(10);
    board.releaseSession()
    console.info('Data');
    console.info(data);
    DataFilter.writeFile(data, 'test.csv', 'w');
    const dataRestored = DataFilter.readFile('test.csv');
    console.info('Data restored');
    console.info(dataRestored);
    DataFilter.writeFile(dataRestored, 'test2.csv', 'w');
}

runExample ();

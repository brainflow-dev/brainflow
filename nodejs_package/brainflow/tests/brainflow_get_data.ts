import {BoardShim} from '../board_shim';
import {BoardIds} from '../brainflow.types';

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
    const data = board.getBoardData();
    board.releaseSession()
    console.info('Data');
    console.info(data);
}

runExample ();

import { BoardShim } from './board-shim';
import { BoardIds } from './brainflow.types';

function sleep(ms: number) {
  return new Promise((resolve) => {
    setTimeout(resolve, ms);
  });
}

async function runExample(): Promise<void> {
  const board = new BoardShim(BoardIds.SYNTHETIC_BOARD, { serialPort: 'test' });

  board.prepareSession();
  board.startStream();

  await sleep(3000);

  board.stopStream();

  const data = board.getBoardData();

  console.info(data);
}

runExample();

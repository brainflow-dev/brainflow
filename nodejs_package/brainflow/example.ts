import { BoardShim } from './board-shim';
import { BoardIds } from './brainflow.types';
import { DataFilter } from './data-filter';

function sleep(ms: number) {
  return new Promise((resolve) => {
    setTimeout(resolve, ms);
  });
}

async function runExample(): Promise<void> {
  const board = new BoardShim(BoardIds.SYNTHETIC_BOARD, { serialPort: 'test' });
  const eegChannels = board.getEegChannels(BoardIds.SYNTHETIC_BOARD);

  board.prepareSession();
  board.startStream();

  await sleep(3000);

  board.stopStream();

  const data = board.getBoardData();
  const railed = eegChannels.map((channel) => DataFilter.getRailedPercentage(data[channel], 24));

  console.info('Data');
  console.info(data[0]);
  console.info('Railed');
  console.info(railed);
}

runExample();

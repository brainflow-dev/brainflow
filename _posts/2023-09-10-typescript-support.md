---
layout: post
title: BrainFlow 5.10.0
subtitle: Typescript support
image: /img/typescript.png
tags: [release]
author: andrey_parfenov
---

We are glad to announce, that in the latest release we added support for Typescript programming language into BrainFlow. Thanks [Gordon Preumont](https://github.com/GordonPmnt) for the help with it!

So, now you can download this package from [npm](https://www.npmjs.com/package/brainflow) and use it to build your own apps on top of it. Detailed installation instructions can be found at [BrainFlow docs](https://brainflow.readthedocs.io/en/stable/BuildBrainFlow.html#typescript).

Here is a well known brainflow get data example adopted for Typescript:

```javascript
import {BoardIds, BoardShim} from 'brainflow';

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
    board.releaseSession();
    console.info('Data');
    console.info(data);
}

runExample ();
```

More code samples can be found [here](https://brainflow.readthedocs.io/en/stable/Examples.html#typescript).

You are welcome to give it a try and report issues if any via BrainFlow slack workspace or Github issues.
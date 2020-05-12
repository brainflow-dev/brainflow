---
layout: post
title: BrainFlow 2.5.2
subtitle: New patch version of BrainFlow
tags: [release]
---

Added method:

```
get_eeg_names ()
```

It returns locations of EEG electrodes in 10-20 BCI system. If board has flexible locations for EEG electrodes this method returns UNSUPPORTED_BOARD_ERROR. So for currently supported boards it works for BrainBit and Unicorn since OpenBCI boards can be connected to everything.

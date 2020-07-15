---
layout: post
title: BrainFlow 3.2.0
image: /img/android.png
tags: [release]
---

Changes:

* *Experimental:* Add [support for Android](https://brainflow.readthedocs.io/en/stable/BuildBrainFlow.html#android) for OpenBCI WIFI Shield and BrainFlow Synthetic Board.
* Link all third party libraries manually in runtime, it will make entire system more robust and modular
* Improved docs for [Unity Integration](https://brainflow.readthedocs.io/en/stable/GameEngines.html#unity)
* Now OpenBCI WIFI shield can be autodiscovered, even if it's connected to a local network

Changes for BrainFlow developers:

* There is no *brainflow_boards.json* anymore, this code was moved to header file
* Improved docs for adding new boards

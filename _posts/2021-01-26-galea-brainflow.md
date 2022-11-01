---
layout: post
title: OpenBCI Galea with BrainFlow
subtitle: OpenBCI Galea device developed in collaboration with Valve corporation will use BrainFlow SDK
image: /img/galea.jpg
tags: [release]
author: andrey_parfenov
---


*"If you’re a software developer in 2022 who doesn’t have one of these in your test lab, you’re making a silly mistake."- [Gabe Newell](https://www.tvnz.co.nz/one-news/new-zealand/gabe-newell-says-brain-computer-interface-tech-allow-video-games-far-beyond-human-meat-peripherals-can-comprehend)*

<div style="text-align: center">
    <a href="https://galea.co/#splash" title="galea" target="_blank" align="center">
        <img width="500" height="442" src="https://live.staticflickr.com/65535/50876030896_a0c78eec4d_z.jpg">
    </a>
</div>


**We are ecstatic to announce that new device developed in collaboration between [OpenBCI](https://openbci.com/community/) and [Valve corporation](https://www.valvesoftware.com/en/) will use [BrainFlow SDK](https://github.com/brainflow-dev/brainflow) as a first choice library.**

[Galea](https://galea.co/) is a hardware platform developed together with Valve company that merges next-generation biometrics with mixed reality. This is the first device that integrates EEG, EMG, EDA, PPG, and eye-tracking into a single headset. The Galea hardware will be integrated into existing AR and VR head-mounted displays. By combining this multi-modal sensor system with the immersion of augmented and virtual reality, Galea gives researchers, developers, and creators a powerful new tool for understanding and augmenting the human mind and body.


OpenBCI is a partner of BrainFlow and uses BrainFlow as a primary SDK to work with all their exising devices and inside [OpenBCI GUI](https://github.com/OpenBCI/OpenBCI_GUI). You can read more about this announcement [here](https://openbci.com/community/brainflowqa/).

We continue working with OpenBCI and help them to integrate new devices like Galea.

**Benefits of BrainFlow which make it great for hardware development and for end users:**

* powerful API with many features to simplify development
    * Straightforward API for data acquisition
    * Powerful API for signal filtering, denoising, downsampling...
    * Development tools like Synthetic board, Streaming board, logging API
* easy to use
    * BrainFlow has many bindings, you can choose programming language you like
    * All programming languages provide the same API, so it's simple to switch
    * API is uniform for all boards, it makes applications on top of BrainFlow almost board agnostic
* easy to support and extend
    * Code to read data and to perform signal processing is implemented only once in C/C++, bindings just call C/C++ methods
    * Powerful CI/CD system which runs integrations tests for each commit automatically using BrainFlow's Emulator
    * Simplified process to add new boards and methods

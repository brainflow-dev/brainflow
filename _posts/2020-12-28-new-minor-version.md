---
layout: post
title: BrainFlow 3.8.0
image: /img/gforcearmband.jpg
tags: [release]
---

### Summary of Changes:

* Add OYMotion gForcePro Armband
* Refactor Julia binding
* Publish Julia Package to the Registry 
* Add OpenMP support for some methods
* Refactor enums in Python binding
* Add `get_device_name` method
* Add more Cpp checks to CI

#### OYMotion gForcePro Armband

<div style="text-align: center">
    <a href="https://github.com/brainflow-dev/brainflow/pull/105" title="OYMotion" target="_blank" align="center">
        <img width="484" height="430" src="https://live.staticflickr.com/65535/50760349443_368326974c_o.jpg">
    </a>
</div>

Device specs:

* 8-Channel real-time raw EMG data acquisition
* 12Bit mode with 500Hz sample rate

#### Julia refactoring and public package

Thanks to [matthijscox](https://github.com/matthijscox) for implementing it. Julia package was refactored and improved significantly, also now it's available for download from Julia Package Registry.

#### OpenMP vs C++11 threads

We realized that OpenMP provides better performance than C++11 threads. So, we moved to OpenMP. Since OpenMP can not be linked statically we had to link it as a dynamic library. At the same time we didn't want to force users to install system packages(like ligbomp, libomp, etc), and we decided to publish precompiled libraries without OpenMP parallelization.

Those who need better performance of signal processing algorithms can compile BrainFlow from the source with OpenMP Enabled.

#### Improve enums in Python bindings

From now on you can write smth like `BoardIds.SYNTHETIC_BOARD` instead `BoardIds.SYNTHETIC_BOARD.value` the same is true for all enums in python binding. Both options will work fine, it's not a breaking change. Thanks to [imachug](https://github.com/imachug).

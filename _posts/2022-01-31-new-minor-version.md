---
layout: post
title: BrainFlow 4.9.0
subtitle: Muse2016 and extended API
image: /img/update.jpg
tags: [release]
author: andrey_parfenov
---


### Summary

* `get_version()` method - [#417](https://github.com/brainflow-dev/brainflow/pull/417)
* `calc_stddev()` method - [#403](https://github.com/brainflow-dev/brainflow/pull/403/files)
* `release_all()` method - [#402](https://github.com/brainflow-dev/brainflow/pull/402)
* Updated SimpleBLE library with bug fixes for Linux
* Support for Muse2016


### Muse 2016

<div style="text-align: center">
    <a href="https://choosemuse.com/muse-s/" title="Muse2016" target="_blank" align="center">
        <img width="500" height="500" src="https://live.staticflickr.com/65535/51854219574_24c42b30d9_z.jpg">
    </a>
</div>

We have added support for Muse2016 with and without BLED112 dongle, feel free to test it and report issues if any.

### MacOS Monterey BLE issues

If you have problems with BLE devices like Muse on MacOS 12.0-12.2 please update MacOS version to 12.3. More info can be found [here](https://github.com/OpenBluetoothToolbox/SimpleBLE/issues/33).

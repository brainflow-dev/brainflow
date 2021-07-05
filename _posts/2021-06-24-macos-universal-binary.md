---
layout: post
title: BrainFlow 4.3.1
subtitle: Add support for Apple M1
image: /img/m1.jpg
tags: [release, macos]
author: andrey_parfenov
---


### Changes in this release:

* [Add support for Apple M1 processors](https://github.com/brainflow-dev/brainflow/issues/290)
* [Fix types for numbers in `get_board_descr` method for Java](https://github.com/brainflow-dev/brainflow/issues/292)
* [Add CI pipelines for Matlab](https://github.com/brainflow-dev/brainflow/issues/286)


### Universal binaries for MacOS

If you have an error like `mach-o but wrong architecture` on Apple M1, upgrade to this version and problem should be resolved.

<div style="text-align: center">
    <a href="https://github.com/brainflow-dev/brainflow/issues/290" title="brainflow_universal" target="_blank" align="center">
        <img width="640" height="233" src="https://live.staticflickr.com/65535/51269306180_f3e2c9e077_z.jpg">
    </a>
</div>

Before 4.3.1 precompiled libraries for MacOS were only for x86_64 arch. In this version we've moved to universal binaries which have native code for both architectures(ARM and X86). We did that only for libraries provided by us and we can not update 3rd party precompiled libraries which we are using for some devices.

More information about universal binaries for MacOS can be found here:

* [Building a Universal macOS Binary](https://developer.apple.com/documentation/apple-silicon/building-a-universal-macos-binary)
* [Porting Your macOS Apps to Apple Silicon](https://developer.apple.com/documentation/apple-silicon/porting-your-macos-apps-to-apple-silicon)

We've tested that such libraries work for universal and non-universal executables and for both architectures.

<div style="text-align: center">
    <a href="https://github.com/brainflow-dev/brainflow/issues/290" title="brainflow_universal" target="_blank" align="center">
        <img width="640" height="266" src="https://live.staticflickr.com/65535/51268277986_df7c84ba80_z.jpg">
    </a>
</div>



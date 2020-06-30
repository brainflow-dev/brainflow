---
layout: post
title: BrainFlow 3.1.0
tags: [release]
---

Breaking changes in this release:

* Methods to calculate FFT now have window argument. Chech code samples
* C++ only: all constants are C++ 11 enum classes now without typedef. It's important to prevent name conflicts

Other changes:

* New methods in signal processing API to calculate PSD and band power
* Fix streaming board on multinode, before it worked only on localhost
* Now we publish Java packages to Github packages. Will add instructions in next posts
* Parallel compilation using bash and batch files
* Minot refactoring in BrainFlow internals

---
layout: post
title: BrainFlow 2.5.0
subtitle: Add support for Unicorn device
image: /img/unicorn.jpg
tags: [release]
---

Added support for [Unicorn Device](https://www.unicorn-bi.com/). Currently it works only for Linux, we are working on Windows support. It means that from now on BrainFlow provides free SDKs(unlike SDKs provided by Unicorn) to work with Unicorn device for all supported languages.

We didn't reverse libraries provided by Unicorn and didn't reverse Unicorn's communications protocol. Unicorn provides C++ SDK for free, we integrated it into BrainFlow's Core Module and didn't violate license agreement. Moreover we don't link BrainFlow with this library, we call dlopen manually instead.
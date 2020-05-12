---
layout: post
title: BrainFlow 2.3.1
subtitle: New patch version of BrainFlow
tags: [release]
---

Hot fix for 2.3.0, on MacOS if System Integrity Protection is enabled Pyhon binding didn't work.

Since 2.3.0 in BrainFlow we have major dynamic library which we load and several libraries which are used by the major one. To ensure that these libraries are in search path we used to set environment variables directly in python process. But suddenly it doesn't work if SIP is enabled. So, since 2.3.1 for MacOS we manually load all dynamic libraries which are used by the major library.
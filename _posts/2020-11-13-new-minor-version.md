---
layout: post
title: BrainFlow 3.7.1
image: /img/freeeeg.png
tags: [release]
---

Changes in this release:

* Add FreeEEG32 for all OSes
* Move to new manylinux image, there is centos 6 and new GCC. Before we used CentOS 5 and GCC 4.8. More information about new manylinux docker container can be found [here](https://www.python.org/dev/peps/pep-0599/)
* Drop support of GCC < 5
* Add support for custom baud rate to Serial class
* Add Serial Number check for Callibri device

Using modern GCC we can also provide compiled C++ libraries for C++ users.
Before there was ABI mismatch between GCC 4.8 and 5.0+

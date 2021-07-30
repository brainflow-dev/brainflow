---
layout: page
title: Roadmap
subtitle: Global Tasks to Accomplish
---


This page provides an overview of the major themes in BrainFlow's development. Each of these items requires a relatively large amount of effort to implement. These may be achieved more quickly with dedicated funding or interest from contributors.

An item being on the roadmap does not mean that it will necessarily happen, even with unlimited funding. During the implementation period we may discover issues preventing the adoption of the feature.

Additionally, an item not being on the roadmap does not exclude it from inclusion in BrainFlow. The roadmap is intended for larger, fundamental changes to the project that are likely to take months or years of developer time. Smaller-scoped items will continue to be tracked on our issue tracker.

## Rust and C++ interoperability

We are experimenting with Rust. For the foreseeable future, C++ is the reigning monarch in our codebase, and any use of Rust will need to fit in with C++ — not the other way around. This seems to present some C++/Rust interoperability challenges which nobody else has faced.

Primary goal is to simplify process of adding new boards using Rust, also we want to make our codebase safer in terms of memory errors. Rust has some [libraries for BT,BLE](https://github.com/deviceplug/btleplug) which are not available for C++. We want to use them to simplify BT,BLE communication in BrainFlow.

## Support as many devices as possible

BrainFlow has built-in framework for adding new devices. It works great and simplifies this process, but to add new boards we need to buy them or collaborate with manufactorers.

At the beginning of BrainFlow we were focused on low-cost DIY boards and devices for researchers and software engineers. Now we are working on integration of medical grade and consumer devices from different price segments and use cases. In the feasible future percentage of consumer and medical grade devices in BrainFlow will grow, and it will be our primary focus.

## Port BrainFlow to new platforms and languages

So far we support:

* Linux
* Windows
* MacOS
* Raspberry Pi(for some devices)
* Android(for some devices)

We are working on adding support for other OSes like IOS and adding more boards to Android and Raspberry Pi.

Also, we are looking forward to add more bindings e.g. JavaScript.

The last but not the least we are integrating BrainFlow to Game Engines and other applications.

## Signal Processing and ML API improvements 

List of potential tasks:

* Implement ICA based signal denoising
* Implement Common Spatial Patterns(CSP)
* Add more ML classifiers for existing derivative metrics
* Calculate more derivate metrics
* Implement P300 classification on the API level
* Add the opportunity to load user's models via [onnxruntime](https://github.com/microsoft/onnxruntime)


## Documentation and website improvements

We'd like to improve the content, structure, and presentation of the BrainFlow documentation. Some specific goals include:

* Improve the "Getting Started" documentation, designing and writing learning paths for users different backgrounds (e.g. brand new to programming, experienced programmers unfamiliar with BCI technology, already familiar with BCI technology or BCI scientists without programming experience).
* Improve Supported Boards section at BrainFlow docs to simplify search for the users
* Improve the overall organization of the documentation and specific subsections of the documentation to make navigation and finding content easier.

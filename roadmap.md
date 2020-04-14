# Roadmap

This page provides an overview of the major themes in BrainFlow's development. Each of these items requires a relatively large amount of effort to implement. These may be achieved more quickly with dedicated funding or interest from contributors.

An item being on the roadmap does not mean that it will necessarily happen, even with unlimited funding. During the implementation period we may discover issues preventing the adoption of the feature.

Additionally, an item not being on the roadmap does not exclude it from inclusion in BrainFlo. The roadmap is intended for larger, fundamental changes to the project that are likely to take months or years of developer time. Smaller-scoped items will continue to be tracked on our issue tracker.

## Support as many devices as possible

We have powerful system for multi board expansion, which allows us to relatively simple add support for new devices.

Currently we added or going to add support for:

* [~~Streaming Board~~](https://brainflow.readthedocs.io/en/stable/SupportedBoards.html#streaming-board)
* [~~Synthetic Board~~](https://brainflow.readthedocs.io/en/stable/SupportedBoards.html#synthetic-board)
* [~~OpenBCI Cyton~~](https://docs.openbci.com/docs/01GettingStarted/01-Boards/CytonGS)
* [~~OpenBCI Ganglion~~](https://docs.openbci.com/docs/01GettingStarted/01-Boards/GanglionGS)
* [~~OpenBCI Cyton Daisy~~](https://docs.openbci.com/docs/01GettingStarted/01-Boards/DaisyGS)
* [~~OpenBCI Ganglion with WIFI~~](https://docs.openbci.com/docs/01GettingStarted/01-Boards/WiFiGS)
* [~~OpenBCI Cyton with WIFI~~](https://docs.openbci.com/docs/01GettingStarted/01-Boards/WiFiGS)
* [~~OpenBCI Cyton Daisy with WIFI~~](https://docs.openbci.com/docs/01GettingStarted/01-Boards/WiFiGS)
* [~~BrainBit~~](https://brainbit.com/)
* [Unicorn](https://www.unicorn-bi.com/)(~~Linux~~, Windows)
* [Muse](https://choosemuse.com/)
* [Callibri](https://callibri.com/)
* [FreeEEG](https://github.com/neuroidss/FreeEEG32)
* [HackEEG](https://www.starcat.io/products/hackeeg-shield/)
* etc

To add new devices to BrainFlow API we need to buy them or work together with manufacturers.

## Port BrainFlow to new platforms and languages

So far we support:

* Linux
* Windows
* MacOS

For selected boards we are going to port BrainFlow to IOS and Android. For certain devices it's technically impossible (e.g. communication via serial port). But for some of them it's doable and will provide new opportunities and attract more users. Signal processing API can be ported to any platform without any limitations. Also, currently we don't provide compiled libraries for devices like Nvidia Jetson Nano, Raspberry Pi, etc, and force users to compile BrainFlow on these devices by themself.

Currently BrainFlow provides bindings for:

* C++
* Java
* Python
* R
* C#
* Julia
* Matlab

We are looking forward to add more languages.

## Signal Processing API Improvement

Existing signal processing API is quite poerful, but there is room for improvement. 

Adding more options for signal denoising and artifacts removal will be really useful. One of the most popular methods for it is ICA based denoising which is not implemented in BrainFlow yet.


## Implement Riemannian geometry for BCI classification

There are many frameworks for machine learning and we are not going to create our own. But in BCI domain Riemannian geometry is one of the most powerful tools for signal classification. Unfortunately most of existing machine learning frameorks don't support it.


## Documentation and website improvements

We'd like to improve the content, structure, and presentation of the BrainFlow documentation. Some specific goals include:

* Improve the "Getting Started" documentation, designing and writing learning paths for users different backgrounds (e.g. brand new to programming, experienced programmers unfamiliar with BCI technology, already familiar with BCI technology or BCI scientists without programming experience).
* Improve the overall organization of the documentation and specific subsections of the documentation to make navigation and finding content easier.

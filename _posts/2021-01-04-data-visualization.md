---
layout: post
title: Realtime biosignal visualization
subtitle: How to use BrainFlow with the gForce Pro
image: /img/gforcearmband.jpg
tags: [tutorial]
---

### Introduction

So you obtained a nifty neurotechnology gadget? And now you would like to work with the data in your favorite development environment? In the past it took me months to achieve this, but thanks to BrainFlow it can work in minutes! Let me show you how I did it.

Most of the code should work for any device, but I will use the OYMotion gForce Pro armband we used in a [past project](https://medium.com/symbionic-project). It streams 8 channels of electromyographic (EMG) data at 600 Hz. I just want to see this data realtime in a plot. In the past I always used Python for this task, but the experience was not smooth, because it had difficulties with handling this high data rate. I could try to write everything in c++, but that would take me ages. I recently found the [Julia language](https://julialang.org/), which is aimed to be both fast and easy to use. With Julia I succeeded with ease, so I'll show you how to visualize the data in Julia. Python is still fine for acquiring data and doing offline analysis, but I would never use it again for such a high performance task on realtime data.

### Installing BrainFlow.jl

Assuming you have Julia installed, you should start by adding the BrainFlow.jl package. Since BrainFlow 3.8.0 we have an automated Julia package, which installs everything for you. Adding packages in Julia is easy with the internal Julia package manager, just type in the Julia REPL:

    using Pkg
    Pkg.add("BrainFlow")

This may take a moment, since it has to download the compiled BrainFlow c++ libraries.

### Starting small

Let's start by acquiring a small data sample and plotting it. We'll get to streaming the data later. 

TODO: script is on other laptop. Also show results in command line from succesful connection?
* Connect to BrainFlow
* Get Data
* Make VegaLite plot

### Streaming data

Let's move on to visualizing the live data. You can see the final result below, where we make a few gestures and watch the biosignals unfold right in front of your eyes. It's always a magical moment!

[![IMAGE ALT TEXT HERE](/img/EMGdataYoutube.jpg)](https://www.youtube.com/watch?v=L7NXYZ5EDdw)

So how did I succeed in doing this? It was a bit more work than making a single plot, so I placed the code in this[BrainFlowViz.jl](https://github.com/matthijscox/BrainFlowViz.jl) repository. 


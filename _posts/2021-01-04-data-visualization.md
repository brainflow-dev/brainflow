---
layout: post
title: Realtime biosignal visualization
subtitle: How to use BrainFlow with the gForce Pro
image: /img/EMGdataPhoto.jpg
tags: [tutorial]
---

### Introduction

So you obtained a nifty neurotechnology gadget? And now you would like to work with the data in your favorite development environment? In the past it took me months to achieve this, but thanks to BrainFlow it can work in minutes! Let me show you how I did it.

Most of the code should work for any device, but I will use the OYMotion gForce Pro armband we used in a [past project](https://medium.com/symbionic-project). It streams 8 channels of electromyographic (EMG) data at 600 Hz. I just want to see this data realtime in a plot. In the past I always used Python for this task, but the experience was not smooth, because it had difficulties with handling this high data rate. I could try to write everything in c++, but that would take me ages. I recently found the [Julia language](https://julialang.org/), which is aimed to be both fast and easy to use. With Julia I succeeded with ease, so I'll show you how to visualize the data in Julia. Python is still fine for acquiring data and doing offline analysis, but I would never use it again for such a high performance task on realtime data.

### Installing BrainFlow.jl

Assuming you have Julia installed, you should start by adding the BrainFlow.jl package. Since BrainFlow 3.8.0 we have an automated Julia package, which installs everything for you. Adding packages in Julia is easy with the internal Julia package manager, just type in the Julia REPL:

```
using Pkg
Pkg.add("BrainFlow")
```

This may take a moment, since it has to download the compiled BrainFlow c++ libraries.

### Starting small

Let's start by acquiring a small data sample and plotting it. We'll get to streaming the data later. 

First we have to connect to the gForce Pro device using BrainFlow. You can find how to do this in other languages in the BrainFlow docs [examples section](https://brainflow.readthedocs.io/en/stable/Examples.html).

```
using BrainFlow
BrainFlow.enable_dev_logger(BrainFlow.BOARD_CONTROLLER)
params = BrainFlowInputParams()
board_shim = BrainFlow.BoardShim(BrainFlow.GFORCE_PRO_BOARD, params)
BrainFlow.prepare_session(board_shim)
BrainFlow.start_stream(board_shim)
```

TODO: show the succesful logged result from the command line.

That's it! I assume the code is pretty self-explanatory.

Once the stream is started, you can get the collected data from BrainFlow directly via:
```
data = BrainFlow.get_board_data(board_shim) |> transpose
```
The output data is a matrix where the columns are the available channels and the rows are the samples. Andrey from BrainFlow tranposed the matrix, because he didn't yet know Julia is column-major, so I transposed it back. We may remove this transpose in a future version, so beware.

This data matrix has 10 channels, while we know the gForce armband has only 8 EMG channels. We can check which of the columns are the EMG channels via the following:
```
julia> BrainFlow.get_emg_channels(board_shim)
2:9
```
The first column is actually a package identifier related to how the gForce sends the data, the 10th column is a timestamp we could also use if we want. The function `BrainFlow.get_timestamp_channel` could have told as that. An overview of which channels are available in which board would be a nice addition to the BrainFlow documentation. Feel free to become an open source contributor ;)

Now let's make a plot from the data. For this single static plot, I'll use the VegaLite.jl package. The plotting eco-system around Julia is still evolving rapidly with Plots.jl, Makie.jl and more, but I like these grammar of graphics style interfaces like ggplot from R. [VegaLite](https://vega.github.io/vega-lite/) provides such a grammar of graphics and [VegaLite.jl](https://github.com/queryverse/VegaLite.jl) is a Julia wrapper around it.

In order to best use VegaLite, we first create a dataframe using DataFrames.jl

TODO: script is on other laptop. 
* Add DataFrames + VegaLite script
* Save a .jpg from a session

### Streaming data visualization

Let's move on to visualizing the live data. You can see the final result below, where we make a few gestures and watch the biosignals unfold right in front of your eyes. It's always a magical moment!

[![IMAGE ALT TEXT HERE](/img/EMGdataYoutube.jpg)](https://www.youtube.com/watch?v=L7NXYZ5EDdw)


#### Parsing the data

So how did I succeed in doing this? It was a bit more work than making a single plot, so I placed the code in this[BrainFlowViz.jl](https://github.com/matthijscox/BrainFlowViz.jl) repository. The gForce visualization script is in [/test/brainflow_gforce.jl](https://github.com/matthijscox/BrainFlowViz.jl/blob/main/test/brainflow_gforce.jl). Let's walk through it.

First make sure the BrainFlow has started a stream, just like in the previous section.

Then I wrote a custom function to acquire some emg data and detrend it.

```
function get_some_board_data(board_shim, nsamples)
    data = BrainFlow.get_current_board_data(nsamples, board_shim)
    data = transpose(data)
    emg_data = view(data, :, 2:9)
    for chan in 1:8
        emg_channel_data = view(emg_data, :, chan)
        BrainFlow.detrend(emg_channel_data, BrainFlow.CONSTANT)
    end
    return emg_data
end
```

The function `BrainFlow.get_current_board_data` is slightly different from `BrainFlow.get_board_data` in that it retrieves only the last n-samples of data and does not clear the internal brainflow buffer. This will be helpful for continuously requesting a slice of data and plotting it. Beware that at the very start of the stream the internal buffer is not yet full, so you may get a matrix with less rows than the requested number of samples.

Next I select the EMG data channels as a `view` for performance, because that creates a kind of reference to the data instead of making a copy.

In a quick for-loop, I then continue to detrend each 'viewed' EMG channel individual with `BrainFlow.detrend()`. A constant detrend means we just remove the average per channel. I could probably have done this efficiently with some Julia code, but wanted to try out calling a brainflow data filtering function from Julia. This function directly passes an array from Julia in-memory to the brainflow c++ libraries. 

As a small interlude, you can read the BrainFlow [julia-package source code](https://github.com/brainflow-dev/brainflow/blob/master/julia-package/brainflow/src/data_filter.jl#L91) you would see just how easy it is to do this in Julia:
```
    ccall((:detrend, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint), data, length(data), Int32(operation))
```
One line of code! I won't go into the details, but I dare you to do these tricks yourself in [Cython](https://cython.org/) or [Rcpp](http://www.rcpp.org/) or MATLAB's MEX framework. This is one of the reason I love Julia, integration with c/c++ is pretty much seamless.

We are now ready to start plotting the data.

#### Plotting the data

I made a nice convenience function for plotting the brainflow data from any data retrieval function. Here's what I used to make my video. Beforehand, I called the `get_some_board_data()` function once while wearing the armband to check what would be good y-scale limits.

```
using BrainFlowViz
nchannels = 8
nsamples = 512
data_func = ()->get_some_board_data(board_shim, nsamples)

BrainFlowViz.plot_data(
    data_func, 
    nsamples, 
    nchannels; 
    y_lim = [-1300 1300], 
    theme = :dark,
    color = :lime,
    )
```

Alright, but how does this work internally? For plotting the animated data I did not use VegaLite.jl because it is not suitable for high performance animations. For that reason I choose [Makie.jl](https://github.com/JuliaPlots/Makie.jl) which calls itself a _high-performance, extendable, and multi-platform plotting ecosystem for the Julia programming language._ It also has an optional WebGL backend in case I want to move to a web-based application.

I used the Makie documentation to find my way. First I needed an array of Nodes or Observables which I fill with the EMG data. Whenever a Node changes, Makie will refresh the plot for you. That's very convenient! Just to be sure we are performant I push views of the EMG data into each Node.

```
ys_n = []
for n = 1:nchannels
    push!(ys_n, Node(view(ys, :, n)))
end
```

using Makie, AbstractPlotting





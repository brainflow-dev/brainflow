#!/usr/bin/env julia
using BrainFlow

include("julia_tests.jl")
include("brainflow_get_data.jl")
include("signal_filtering.jl")
include("denoising.jl")
include("downsampling.jl")
#include("csp.jl") # currently getting an error that get_csp cannot be loaded from libraries 3.9.2
include("serialization.jl")
include("transforms.jl")
include("eeg_metrics.jl")
include("markers.jl")
include("band_power.jl")
include("band_power_all.jl")
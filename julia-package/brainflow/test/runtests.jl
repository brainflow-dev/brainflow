#!/usr/bin/env julia
using brainflow

include("brainflow_get_data.jl")
include("signal_filtering.jl")
include("denoising.jl")
include("downsampling.jl")
include("serialization.jl")
include("transforms.jl")
include("eeg_metrics.jl")
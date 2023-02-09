#!/usr/bin/env julia
using BrainFlow

include("julia_tests.jl")
include("brainflow_get_data.jl")
include("brainflow_get_data_twice.jl")
include("signal_filtering.jl")
include("denoising.jl")
include("downsampling.jl")
include("csp.jl")
include("serialization.jl")
include("transforms.jl")
include("markers.jl")
include("release_all.jl")
include("spo2.jl")
include("ica.jl")

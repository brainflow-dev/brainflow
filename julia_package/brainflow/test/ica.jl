using BrainFlow

# Two simultaneous mixtures: rows are channels and columns are samples.
sample_times = collect(0:1023) ./ 256.0
source1 = sin.(2.0 .* pi .* 7.0 .* sample_times)
source2 = sin.(2.0 .* pi .* 13.0 .* sample_times) .^ 3
data = permutedims(hcat(source1 .+ 0.3 .* source2, 0.2 .* source1 .+ source2))
w, k, a, sources = BrainFlow.perform_ica(data, 2)
# Component order and sign are arbitrary.
println("Recovered $(size(sources, 1)) sources from $(size(sources, 2)) samples")

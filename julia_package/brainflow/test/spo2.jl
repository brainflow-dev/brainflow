using BrainFlow

# Demonstrate calibrated, shared red/IR pulsatility using 15 seconds of synthetic PPG.
# Keep enough surrounding samples for filtering plus at least four seconds of analysis.
sampling_rate = 128
sample_times = collect(0:(15 * sampling_rate - 1)) ./ sampling_rate
pulse = sin.(2.0 .* pi .* 1.2 .* sample_times) .+ 0.15 .* sin.(2.0 .* pi .* 2.4 .* sample_times)
data_ir = 100000.0 .+ 1000.0 .* pulse
data_red = 80000.0 .+ 400.0 .* pulse

# Real sensors require their own calibrated coefficients and good optical contact.
spo2 = BrainFlow.get_oxygen_level(data_ir, data_red, sampling_rate)
println("Synthetic SpO2: $spo2")

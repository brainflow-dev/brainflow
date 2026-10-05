library(brainflow)

# Two simultaneous mixtures: rows are channels and columns are samples.
sample_times <- (0:1023) / 256
source1 <- sin(2 * pi * 7 * sample_times)
source2 <- sin(2 * pi * 13 * sample_times)^3
mixed_data <- rbind(source1 + 0.3 * source2, 0.2 * source1 + source2)
numpy_data <- np$array(mixed_data, dtype = np$float64, order = "C")
# Component order and sign are arbitrary.
ica <- brainflow_python$DataFilter$perform_ica(numpy_data, as.integer(2))

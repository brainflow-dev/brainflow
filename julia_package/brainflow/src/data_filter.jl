@enum AggOperations begin

    MEAN = 0
    MEDIAN = 1
    EACH = 2

end

AggType = Union{AggOperations, Integer}

@enum FilterTypes begin

    BUTTERWORTH = 0
    CHEBYSHEV_TYPE_1 = 1
    BESSEL = 2
    BUTTERWORTH_ZERO_PHASE = 3
    CHEBYSHEV_TYPE_1_ZERO_PHASE = 4
    BESSEL_ZERO_PHASE = 5

end

FiltType = Union{FilterTypes, Integer}

@enum WindowOperations begin

    NO_WINDOW = 0
    HANNING = 1
    HAMMING = 2
    BLACKMAN_HARRIS = 3

end

WinType = Union{WindowOperations, Integer}

@enum DetrendOperations begin

    NO_DETREND = 0
    CONSTANT = 1
    LINEAR = 2

end

DetType = Union{DetrendOperations, Integer}

@enum NoiseTypes begin

    FIFTY = 0
    SIXTY = 1
    FIFTY_AND_SIXTY = 2

end

EnvNoiseType = Union{NoiseTypes, Integer}

@enum WaveletDenoisingTypes begin

    VISUSHRINK = 0
    SURESHRINK = 1

end

WaveletDenoisingType = Union{WaveletDenoisingTypes, Integer}

@enum ThresholdTypes begin

    SOFT = 0
    HARD = 1

end

ThresholdType = Union{ThresholdTypes, Integer}

@enum WaveletExtensionTypes begin

    SYMMETRIC = 0
    PERIODIC = 1

end

WaveletExtensionType = Union{WaveletExtensionTypes, Integer}

@enum NoiseEstimationLevelTypes begin

    FIRST_LEVEL = 0
    ALL_LEVELS = 1

end

NoiseEstimationLevelType = Union{NoiseEstimationLevelTypes, Integer}

@enum WaveletTypes begin

    HAAR = 0
    DB1 = 1
    DB2 = 2
    DB3 = 3
    DB4 = 4
    DB5 = 5
    DB6 = 6
    DB7 = 7
    DB8 = 8
    DB9 = 9
    DB10 = 10
    DB11 = 11
    DB12 = 12
    DB13 = 13
    DB14 = 14
    DB15 = 15
    BIOR1_1 = 16
    BIOR1_3 = 17
    BIOR1_5 = 18
    BIOR2_2 = 19
    BIOR2_4 = 20
    BIOR2_6 = 21
    BIOR2_8 = 22
    BIOR3_1 = 23
    BIOR3_3 = 24
    BIOR3_5 = 25
    BIOR3_7 = 26
    BIOR3_9 = 27
    BIOR4_4 = 28
    BIOR5_5 = 29
    BIOR6_8 = 30
    COIF1 = 31
    COIF2 = 32
    COIF3 = 33
    COIF4 = 34
    COIF5 = 35
    SYM2 = 36
    SYM3 = 37
    SYM4 = 38
    SYM5 = 39
    SYM6 = 40
    SYM7 = 41
    SYM8 = 42
    SYM9 = 43
    SYM10 = 44

end

WaveletType = Union{WaveletTypes, Integer}


@brainflow_rethrow function perform_lowpass(data, sampling_rate::Integer, cutoff::Float64, order::Integer,
    filter_type::FiltType, ripple::Float64)
    ccall((:perform_lowpass, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Float64, Cint, Cint, Float64),
            data, length(data), Int32(sampling_rate), Float64(cutoff), Int32(order), Int32(filter_type), Float64(ripple))
    return
end

@brainflow_rethrow function perform_highpass(data, sampling_rate::Integer, cutoff::Float64, order::Integer,
    filter_type::FiltType, ripple::Float64
)
    ccall((:perform_highpass, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Float64, Cint, Cint, Float64),
            data, length(data), Int32(sampling_rate), Float64(cutoff), Int32(order), Int32(filter_type), Float64(ripple))
    return
end

@brainflow_rethrow function perform_bandpass(data, sampling_rate::Integer, start_freq::Float64,
    stop_freq::Float64, order::Integer, filter_type::FiltType, ripple::Float64)
    ccall((:perform_bandpass, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Float64, Float64, Cint, Cint, Float64),
            data, length(data), Int32(sampling_rate), Float64(start_freq), Float64(stop_freq), Int32(order), Int32(filter_type), Float64(ripple))
    return
end

@brainflow_rethrow function perform_bandstop(data, sampling_rate::Integer, start_freq::Float64,
    stop_freq::Float64, order::Integer, filter_type::FiltType, ripple::Float64)
    ccall((:perform_bandstop, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Float64, Float64, Cint, Cint, Float64),
            data, length(data), Int32(sampling_rate), Float64(start_freq), Float64(stop_freq), Int32(order), Int32(filter_type), Float64(ripple))
    return
end

@brainflow_rethrow function remove_environmental_noise(data, sampling_rate::Integer, noise_type::EnvNoiseType)
    ccall((:remove_environmental_noise, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Cint),
            data, length(data), Int32(sampling_rate), Int32(noise_type))
    return
end

@brainflow_rethrow function perform_rolling_filter(data, period::Integer, operation::AggType)
    ccall((:perform_rolling_filter, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Cint),
            data, length(data), Int32(period), Int32(operation))
    return
end

@brainflow_rethrow function detrend(data, operation::DetType)
    ccall((:detrend, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint),
            data, length(data), Int32(operation))
    return
end

@brainflow_rethrow function restore_data_from_wavelet_detailed_coeffs(data, wavelet::WaveletType,
                                                                      decomposition_level::Integer,
                                                                      level_to_restore::Integer)
    restored_data = Vector{Float64}(undef, length(data))
    ccall((:restore_data_from_wavelet_detailed_coeffs, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Cint, Cint, Ptr{Float64}),
            data, length(data), Int32(wavelet), Int32(decomposition_level), Int32(level_to_restore), restored_data)
    return restored_data
end

@brainflow_rethrow function detect_peaks_z_score(data, lag::Integer,
                                                 threshold::Float64,
                                                 influence::Float64)
    peaks = Vector{Float64}(undef, length(data))
    ccall((:detect_peaks_z_score, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Float64, Float64, Ptr{Float64}),
            data, length(data), Int32(lag), threshold, influence, peaks)
    return peaks
end

@brainflow_rethrow function perform_wavelet_denoising(data, wavelet::WaveletType, decomposition_level::Integer,
                                                      wavelet_denoising::WaveletDenoisingType,
                                                      threshold::ThresholdType,
                                                      extension::WaveletExtensionType,
                                                      noise_level::NoiseEstimationLevelType)
    ccall((:perform_wavelet_denoising, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Cint, Cint, Cint, Cint, Cint),
            data, length(data), Int32(wavelet), Int32(decomposition_level), Int32(wavelet_denoising),
            Int32(threshold), Int32(extension), Int32(noise_level))
    return
end

@brainflow_rethrow function perform_downsampling(data, period::Integer, operation::AggType)
    len = Integer(floor(length(data) / period))
    downsampled_data = Vector{Float64}(undef, len)
    ccall((:perform_downsampling, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Cint, Ptr{Float64}),
            data, length(data), Int32(period), Int32(operation), downsampled_data)
    return downsampled_data
end

@brainflow_rethrow function write_file(data, file_name::String, file_mode::String)
    shape = size(data)
    flatten = transpose(vcat(data))
    flaten = reshape(flatten, (1, shape[1] * shape[2]))
    flatten = copy(flatten)
    ccall((:write_file, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Ptr{UInt8}, Ptr{UInt8}),
            flatten, shape[1], shape[2], file_name, file_mode)
    return
end

@brainflow_rethrow function get_nearest_power_of_two(value::Integer)
    power_of_two = Vector{Cint}(undef, 1)
    ccall((:get_nearest_power_of_two, DATA_HANDLER_INTERFACE), Cint, (Cint, Ptr{Cint}),
                Int32(value), power_of_two)
    return power_of_two[1]
end

@brainflow_rethrow function calc_stddev(data)
    output = Vector{Float64}(undef, 1)
    ccall((:calc_stddev, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Ptr{Float64}),
                data, 0, length(data), output)
    return output[1]
end

@brainflow_rethrow function get_railed_percentage(data, gain::Integer)
    output = Vector{Float64}(undef, 1)
    ccall((:get_railed_percentage, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Ptr{Float64}),
                data, length(data), gain, output)
    return output[1]
end

@brainflow_rethrow function get_oxygen_level(ppg_ir, ppg_red, sampling_rate::Integer, coef1=1.5958422, coef2=-34.6596622, coef3=112.6898759)
    if length(ppg_ir) != length(ppg_red)
      throw(BrainFlowError(string("invalid size", INVALID_ARGUMENTS_ERROR), Integer(INVALID_ARGUMENTS_ERROR)))
    end

    output = Vector{Float64}(undef, 1)
    ccall((:get_oxygen_level, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Ptr{Float64}, Cint, Cint, Float64, Float64, Float64, Ptr{Float64}),
                ppg_ir, ppg_red, length(ppg_ir), Int32(sampling_rate), coef1, coef2, coef3, output)
    return output[1]
end

@brainflow_rethrow function get_heart_rate(ppg_ir, ppg_red, sampling_rate::Integer, fft_size::Integer)
    if length(ppg_ir) != length(ppg_red)
      throw(BrainFlowError(string("invalid size", INVALID_ARGUMENTS_ERROR), Integer(INVALID_ARGUMENTS_ERROR)))
    end

    output = Vector{Float64}(undef, 1)
    ccall((:get_heart_rate, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Ptr{Float64}, Cint, Cint, Cint, Ptr{Float64}),
                ppg_ir, ppg_red, length(ppg_ir), Int32(sampling_rate), Int32(fft_size), output)
    return output[1]
end

@brainflow_rethrow function get_num_elements_in_file(file_name::String)
    num_elements = Vector{Cint}(undef, 1)
    ccall((:get_num_elements_in_file, DATA_HANDLER_INTERFACE), Cint, (Ptr{UInt8}, Ptr{Cint}),
                file_name, num_elements)
    return num_elements
end

@brainflow_rethrow function read_file(file_name::String)
    num_elements = get_num_elements_in_file(file_name)
    data = Vector{Float64}(undef, num_elements[1])
    num_cols = Vector{Cint}(undef, 1)
    num_rows = Vector{Cint}(undef, 1)
    ccall((:read_file, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Ptr{Cint}, Ptr{Cint}, Ptr{UInt8}, Cint),
                data, num_rows, num_cols, file_name, num_elements[1])
    value = transpose(reshape(data, (num_cols[1], num_rows[1])))
    return value
end

@brainflow_rethrow function perform_wavelet_transform(data, wavelet::WaveletType, decomposition_level::Integer, extension::WaveletExtensionType)
    wavelet_coeffs = Vector{Float64}(undef, length(data) + 2 * (40 + 1))
    lengths = Vector{Cint}(undef, decomposition_level + 1)
    ccall((:perform_wavelet_transform, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Cint, Cint, Ptr{Float64}, Ptr{Cint}),
            data, length(data), Int32(wavelet), Int32(decomposition_level), Int32(extension), wavelet_coeffs, lengths)
    return wavelet_coeffs[1:sum(lengths)], lengths
end


@brainflow_rethrow function perform_inverse_wavelet_transform(wavelet_output, original_data_len::Integer, wavelet::WaveletType, decomposition_level::Integer, extension::WaveletExtensionType)
    original_data = Vector{Float64}(undef, original_data_len)
    ccall((:perform_inverse_wavelet_transform, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Cint, Cint, Ptr{Float64}, Ptr{Float64}),
            wavelet_output[1], Int32(original_data_len), Int32(wavelet), Int32(decomposition_level), Int32(extension), wavelet_output[2], original_data)
    return original_data
end

@brainflow_rethrow function get_csp(data, labels)
    n_epochs = size(data, 1)
    n_channels = size(data, 2)
    n_times = size(data, 3)

    temp_data1d = Vector{Float64}(undef, Integer(n_epochs * n_channels * n_times))
    for e=1:n_epochs
        for c=1:n_channels
            for t=1:n_times
                temp_data1d[(e-1) * n_channels * n_times + (c-1) * n_times + t] = data[e, c, t]
            end
        end
    end

    temp_filters = Vector{Float64}(undef, Integer(n_channels * n_channels))
    output_eigenvalues = Vector{Float64}(undef, Integer(n_channels))

    ccall((:get_csp, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Ptr{Float64}, Cint, Cint, Cint, Ptr{Float64}, Ptr{Float64}), temp_data1d, labels, Int32(n_epochs), Int32(n_channels), Int32(n_times), temp_filters, output_eigenvalues)

    output_filters = Array{Float64,2}(undef, n_channels, n_channels)
    for i=1:n_channels
        for j=1:n_channels
            output_filters[i, j] = temp_filters[(i-1) * n_channels + j]
        end
    end
    return output_filters, output_eigenvalues
end

@brainflow_rethrow function get_window(window_function::WinType, window_len::Integer)
    window_data = Vector{Float64}(undef, Integer(window_len))
    ccall((:get_window, DATA_HANDLER_INTERFACE), Cint, (Cint, Cint, Ptr{Float64}),
    Int32(window_function), Int32(window_len), window_data)
    return window_data
end

@brainflow_rethrow function perform_fft(data, window::WinType)

    if (length(data) % 2 == 1)
        throw(BrainFlowError(string("Data Len must be even ", INVALID_ARGUMENTS_ERROR), Integer(INVALID_ARGUMENTS_ERROR)))
    end

    temp_re = Vector{Float64}(undef, Integer(length(data) / 2) + 1)
    temp_im = Vector{Float64}(undef, Integer(length(data) / 2) + 1)
    res = Vector{Complex}(undef, Integer(length(data) / 2) + 1)

    ccall((:perform_fft, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Ptr{Float64}, Ptr{Float64}),
            data, length(data), Int32(window), temp_re, temp_im)
    for i in 1:Integer(length(data) / 2) + 1
        res[i] = Complex(temp_re[i], temp_im[i])
    end
    return res
end

@brainflow_rethrow function perform_ifft(data)

    temp_re = Vector{Float64}(undef, length(data))
    temp_im = Vector{Float64}(undef, length(data))
    res = Vector{Float64}(undef, 2 * (length(data) - 1))

    for i in 1:length(data)
        temp_re[i] = data[i].re
        temp_im[i] = data[i].im
    end

    ccall((:perform_ifft, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Ptr{Float64}, Cint, Ptr{Float64}),
            temp_re, temp_im, length(res), res)
    return res
end

function get_avg_band_powers(data, channels, sampling_rate::Integer, apply_filter::Bool)
    bands = [(2.0, 4.0), (4.0, 8.0), (8.0, 13.0), (13.0, 30.0), (30.0, 45.0)]
    return get_custom_band_powers(data, bands, channels, sampling_rate, apply_filter)
end

@brainflow_rethrow function get_custom_band_powers(data, bands, channels, sampling_rate::Integer, apply_filter::Bool)

    shape = size(data)
    data_1d = reshape(transpose(data[channels,:]), (1, size(channels)[1] * shape[2]))
    data_1d = copy(data_1d)

    start_freqs = [first(p) for p in bands]
    stop_freqs = [last(p) for p in bands]

    temp_avgs = Vector{Float64}(undef, length(start_freqs))
    temp_stddevs = Vector{Float64}(undef, length(start_freqs))

    ccall((:get_custom_band_powers, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Ptr{Float64}, Ptr{Float64}, Cint, Cint, Cint, Ptr{Float64}, Ptr{Float64}),
            data_1d, size(channels)[1], shape[2], start_freqs, stop_freqs, length(start_freqs), Int32(sampling_rate), Int32(apply_filter), temp_avgs, temp_stddevs)
    return temp_avgs, temp_stddevs
end

@brainflow_rethrow function perform_ica_select_channels(data, num_components::Integer, channels)
    shape = size(data)
    data_1d = reshape(transpose(data[channels,:]), (1, size(channels)[1] * shape[2]))
    data_1d = copy(data_1d)

    temp_w = Vector{Float64}(undef, num_components * num_components)
    temp_k = Vector{Float64}(undef, length(channels) * num_components)
    temp_a = Vector{Float64}(undef, num_components * length(channels))
    temp_s = Vector{Float64}(undef, num_components * shape[2])

    ccall((:perform_ica, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Cint, Ptr{Float64}, Ptr{Float64}, Ptr{Float64}, Ptr{Float64}),
            data_1d, size(channels)[1], shape[2], num_components, temp_w, temp_k, temp_a, temp_s)
    w = transpose(reshape(temp_w, (num_components, num_components)))
    k = transpose(reshape(temp_k, (length(channels), num_components)))
    a = transpose(reshape(temp_a, (num_components, length(channels))))
    s = transpose(reshape(temp_s, (shape[2], num_components)))
    return w, k, a, s
end

function perform_ica(data, num_components::Integer)
    channels = Array((1:size(data)[1]))
    return perform_ica_select_channels(data, num_components, channels)
end

@brainflow_rethrow function get_psd(data, sampling_rate::Integer, window::WinType)

    if (length(data) % 2 == 1)
        throw(BrainFlowError(string("Data Len must be even ", INVALID_ARGUMENTS_ERROR), Integer(INVALID_ARGUMENTS_ERROR)))
    end

    temp_ampls = Vector{Float64}(undef, Integer(length(data) / 2) + 1)
    temp_freqs = Vector{Float64}(undef, Integer(length(data) / 2) + 1)

    ccall((:get_psd, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Cint, Ptr{Float64}, Ptr{Float64}),
            data, length(data), Int32(sampling_rate), Int32(window), temp_ampls, temp_freqs)
    return temp_ampls, temp_freqs
end

@brainflow_rethrow function get_psd_welch(data, nfft::Integer, overlap::Integer, sampling_rate::Integer, window::WinType)

    if (length(data) % 2 == 1)
        throw(BrainFlowError(string("Data Len must be even ", INVALID_ARGUMENTS_ERROR), Integer(INVALID_ARGUMENTS_ERROR)))
    end

    temp_ampls = Vector{Float64}(undef, Integer(nfft / 2) + 1)
    temp_freqs = Vector{Float64}(undef, Integer(nfft / 2) + 1)

    ccall((:get_psd_welch, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Cint, Cint, Cint, Ptr{Float64}, Ptr{Float64}),
            data, length(data), Int32(nfft), Int32(overlap), Int32(sampling_rate), Int32(window), temp_ampls, temp_freqs)
    return temp_ampls, temp_freqs
end

@brainflow_rethrow function get_band_power(psd, freq_start::Float64, freq_end::Float64)
    band_power = Vector{Float64}(undef, 1)
    ccall((:get_band_power, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Ptr{Float64}, Cint, Float64, Float64, Ptr{Float64}),
            psd[1], psd[2], length(psd[1]), Float64(freq_start), Float64(freq_end), band_power)
    return band_power[1]
end

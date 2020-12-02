
@enum AggOperations begin

    MEAN = 0
    MEDIAN = 1
    EACH = 2

end


@enum FilterTypes begin

    BUTTERWORTH = 0
    CHEBYSHEV_TYPE_1 = 1
    BESSEL = 2

end


@enum WindowFunctions begin

    NO_WINDOW = 0
    HANNING = 1
    HAMMING = 2
    BLACKMAN_HARRIS = 3

end


@enum DetrendOperations begin

    # NONE = 0 # will lead to invalid enum redefinitions since enums entries are global constants in fact
    # its a temp hack, lets wait for fixed enums in julia
    CONSTANT = 1
    LINEAR = 2

end


function perform_lowpass(data, sampling_rate::Integer, cutoff::Float64, order::Integer,
    filter_type::Integer, ripple::Float64)
    ec = STATUS_OK
    ec = ccall((:perform_lowpass, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Float64, Cint, Cint, Float64),
            data, length(data), Int32(sampling_rate), Float64(cutoff), Int32(order), Int32(filter_type), Float64(ripple))
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in perform_lowpass ", ec), ec))
    end
    return
end


function perform_highpass(data, sampling_rate::Integer, cutoff::Float64, order::Integer,
    filter_type::Integer, ripple::Float64)
    ec = STATUS_OK
    ec = ccall((:perform_highpass, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Float64, Cint, Cint, Float64),
            data, length(data), Int32(sampling_rate), Float64(cutoff), Int32(order), Int32(filter_type), Float64(ripple))
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in perform_highpass ", ec), ec))
    end
    return
end


function perform_bandpass(data, sampling_rate::Integer, center_freq::Float64,
    band_width::Float64, order::Integer, filter_type::Integer, ripple::Float64)
    ec = STATUS_OK
    ec = ccall((:perform_bandpass, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Float64, Float64, Cint, Cint, Float64),
            data, length(data), Int32(sampling_rate), Float64(center_freq), Float64(band_width), Int32(order), Int32(filter_type), Float64(ripple))
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in perform_bandpass ", ec), ec))
    end
    return
end


function perform_bandstop(data, sampling_rate::Integer, center_freq::Float64,
    band_width::Float64, order::Integer, filter_type::Integer, ripple::Float64)
    ec = STATUS_OK
    ec = ccall((:perform_bandstop, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Float64, Float64, Cint, Cint, Float64),
            data, length(data), Int32(sampling_rate), Float64(center_freq), Float64(band_width), Int32(order), Int32(filter_type), Float64(ripple))
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in perform_bandstop ", ec), ec))
    end
    return
end


function perform_rolling_filter(data, period::Integer, operation::Integer)
    ec = STATUS_OK
    ec = ccall((:perform_rolling_filter, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Cint),
            data, length(data), Int32(period), Int32(operation))
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in perform_rolling_filter ", ec), ec))
    end
    return
end


function detrend(data, operation::Integer)
    ec = STATUS_OK
    ec = ccall((:detrend, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint),
            data, length(data), Int32(operation))
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in detrend ", ec), ec))
    end
    return
end


function perform_wavelet_denoising(data, wavelet::String, decomposition_level::Integer)
    ec = STATUS_OK
    ec = ccall((:perform_wavelet_denoising, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Ptr{UInt8}, Cint),
            data, length(data), wavelet, Int32(decomposition_level))
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in perform_wavelet_denoising ", ec), ec))
    end
    return
end

function perform_downsampling(data, period::Integer, operation::Integer)
    len = Integer(floor(length(data) / period))
    downsampled_data = Vector{Float64}(undef, len)
    ec = STATUS_OK
    ec = ccall((:perform_downsampling, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Cint, Ptr{Float64}),
            data, length(data), Int32(period), Int32(operation), downsampled_data)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in perform_downsampling ", ec), ec))
    end
    downsampled_data
end


function write_file(data, file_name::String, file_mode::String)
    shape = size(data)
    flatten = transpose(vcat(data))
    flaten = reshape(flatten, (1, shape[1] * shape[2]))
    flatten = copy(flatten)
    ec = STATUS_OK
    ec = ccall((:write_file, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Ptr{UInt8}, Ptr{UInt8}),
            flatten, shape[1], shape[2], file_name, file_mode)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in write_file ", ec), ec))
    end
    return
end


function get_nearest_power_of_two(value::Integer)
    power_of_two = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    ec = ccall((:get_nearest_power_of_two, DATA_HANDLER_INTERFACE), Cint, (Cint, Ptr{Cint}),
                Int32(value), power_of_two)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get_nearest_power_of_two ", ec), ec))
    end

    power_of_two[1]
end


function read_file(file_name::String)
    num_elements = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    ec = ccall((:get_num_elements_in_file, DATA_HANDLER_INTERFACE), Cint, (Ptr{UInt8}, Ptr{Cint}),
                file_name, num_elements)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get_num_elements_in_file ", ec), ec))
    end
    
    data = Vector{Float64}(undef, num_elements[1])
    num_cols = Vector{Cint}(undef, 1)
    num_rows = Vector{Cint}(undef, 1)
    ec = ccall((:read_file, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Ptr{Cint}, Ptr{Cint}, Ptr{UInt8}, Cint),
                data, num_rows, num_cols, file_name, num_elements[1])
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in read_file ", ec), ec))
    end
    value = transpose(reshape(data, (num_cols[1], num_rows[1])))
    value
end


function perform_wavelet_transform(data, wavelet::String, decomposition_level::Integer)
    wavelet_coeffs = Vector{Float64}(undef, length(data) + 2 * (40 + 1))
    lengths = Vector{Cint}(undef, decomposition_level + 1)
    ec = STATUS_OK
    ec = ccall((:perform_wavelet_transform, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Ptr{UInt8}, Cint, Ptr{Float64}, Ptr{Cint}),
            data, length(data), wavelet, Int32(decomposition_level), wavelet_coeffs, lengths)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in perform_wavelet_transform ", ec), ec))
    end
    wavelet_coeffs[1:sum(lengths)], lengths
end


function perform_inverse_wavelet_transform(wavelet_output, original_data_len::Integer, wavelet::String, decomposition_level::Integer)
    original_data = Vector{Float64}(undef, original_data_len)
    ec = STATUS_OK
    ec = ccall((:perform_inverse_wavelet_transform, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Ptr{UInt8}, Cint, Ptr{Float64}, Ptr{Float64}),
            wavelet_output[1], Int32(original_data_len), wavelet, Int32(decomposition_level), wavelet_output[2], original_data)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in perform_inverse_wavelet_transform ", ec), ec))
    end
    original_data
end


function perform_fft(data, window::Integer)

    function is_power_of_two(value)
        (value != 0) && (value & (value - 1) == 0)
    end

    if !is_power_of_two(length(data))
        throw(BrainFlowError(string("Data Len must be power of two ", INVALID_ARGUMENTS_ERROR), INVALID_ARGUMENTS_ERROR))
    end

    temp_re = Vector{Float64}(undef, Integer(length(data) / 2) + 1)
    temp_im = Vector{Float64}(undef, Integer(length(data) / 2) + 1)
    res = Vector{Complex}(undef, Integer(length(data) / 2) + 1)

    ec = STATUS_OK
    ec = ccall((:perform_fft, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Ptr{Float64}, Ptr{Float64}),
            data, length(data), Int32(window), temp_re, temp_im)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in perform_fft ", ec), ec))
    end
    for i in 1:Integer(length(data) / 2) + 1
        res[i] = Complex(temp_re[i], temp_im[i])
    end
    res
end


function perform_ifft(data)

    temp_re = Vector{Float64}(undef, length(data))
    temp_im = Vector{Float64}(undef, length(data))
    res = Vector{Float64}(undef, 2 * (length(data) - 1))

    for i in 1:length(data)
        temp_re[i] = data[i].re
        temp_im[i] = data[i].im
    end
    ec = STATUS_OK
    ec = ccall((:perform_ifft, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Ptr{Float64}, Cint, Ptr{Float64}),
            temp_re, temp_im, length(res), res)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in perform_ifft ", ec), ec))
    end
    res
end


function get_avg_band_powers(data, channels, sampling_rate::Integer, apply_filter::Bool)

    shape = size(data)
    data_1d = reshape(transpose(data[channels,:]), (1, size(channels)[1] * shape[2]))
    data_1d = copy(data_1d)

    temp_avgs = Vector{Float64}(undef, 5)
    temp_stddevs = Vector{Float64}(undef, 5)

    ec = STATUS_OK
    ec = ccall((:get_avg_band_powers, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Cint, Cint, Ptr{Float64}, Ptr{Float64}),
            data_1d, size(channels)[1], shape[2], Int32(sampling_rate), Int32(apply_filter), temp_avgs, temp_stddevs)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get_avg_band_powers ", ec), ec))
    end
    temp_avgs, temp_stddevs
end


function get_psd(data, sampling_rate::Integer, window::Integer)

    function is_power_of_two(value)
        (value != 0) && (value & (value - 1) == 0)
    end

    if !is_power_of_two(length(data))
        throw(BrainFlowError(string("Data Len must be power of two ", INVALID_ARGUMENTS_ERROR), INVALID_ARGUMENTS_ERROR))
    end

    temp_ampls = Vector{Float64}(undef, Integer(length(data) / 2) + 1)
    temp_freqs = Vector{Float64}(undef, Integer(length(data) / 2) + 1)

    ec = STATUS_OK
    ec = ccall((:get_psd, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Cint, Ptr{Float64}, Ptr{Float64}),
            data, length(data), Int32(sampling_rate), Int32(window), temp_ampls, temp_freqs)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get_psd ", ec), ec))
    end
    temp_ampls, temp_freqs
end


function get_psd_welch(data, nfft::Integer, overlap::Integer, sampling_rate::Integer, window::Integer)

    function is_power_of_two(value)
        (value != 0) && (value & (value - 1) == 0)
    end

    if !is_power_of_two(nfft)
        throw(BrainFlowError(string("nfft must be power of two ", INVALID_ARGUMENTS_ERROR), INVALID_ARGUMENTS_ERROR))
    end

    temp_ampls = Vector{Float64}(undef, Integer(nfft / 2) + 1)
    temp_freqs = Vector{Float64}(undef, Integer(nfft / 2) + 1)

    ec = STATUS_OK
    ec = ccall((:get_psd_welch, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Cint, Cint, Cint, Ptr{Float64}, Ptr{Float64}),
            data, length(data), Int32(nfft), Int32(overlap), Int32(sampling_rate), Int32(window), temp_ampls, temp_freqs)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get_psd_welch ", ec), ec))
    end
    temp_ampls, temp_freqs
end


function get_band_power(psd, freq_start::Float64, freq_end::Float64)
    band_power = Vector{Float64}(undef, 1)

    ec = STATUS_OK
    ec = ccall((:get_band_power, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Ptr{Float64}, Cint, Float64, Float64, Ptr{Float64}),
            psd[1], psd[2], length(psd[1]), Float64(freq_start), Float64(freq_end), band_power)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get_band_power ", ec), ec))
    end
    band_power[1]
end
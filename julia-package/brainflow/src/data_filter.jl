
abstract type AggOperation <: Integer end
Base.Int32(d::AggOperation) = Int32(Integer(d))
struct Mean <: AggOperation end
struct Median <: AggOperation end
struct Each <: AggOperation end
Base.Integer(::Mean) = 0
Base.Integer(::Median) = 1
Base.Integer(::Each) = 2
const MEAN = Mean()
const MEDIAN = Median()
const EACH = Each()

abstract type FilterType <: Integer end
Base.Int32(d::FilterType) = Int32(Integer(d))
struct Butterworth <: FilterType end
struct Chebyshev <: FilterType end
struct Bessel <: FilterType end
Base.Integer(::Butterworth) = 0
Base.Integer(::Chebyshev) = 1
Base.Integer(::Bessel) = 2
const BUTTERWORTH = Butterworth()
const CHEBYSHEV_TYPE_1 = Chebyshev()
const BESSEL = Bessel()

abstract type WindowFunction <: Integer end
Base.Int32(d::WindowFunction) = Int32(Integer(d))
struct NoWindow <: WindowFunction end
struct Hanning <: WindowFunction end
struct Hamming <: WindowFunction end
struct BlackmanHarris <: WindowFunction end
Base.Integer(::NoWindow) = 0
Base.Integer(::Hanning) = 1
Base.Integer(::Hamming) = 2
Base.Integer(::BlackmanHarris) = 3
const NO_WINDOW = NoWindow()
const HANNING = Hanning()
const HAMMING = Hamming()
const BLACKMAN_HARRIS = BlackmanHarris()

abstract type DetrendOperation <: Integer end
Base.Int32(d::DetrendOperation) = Int32(Integer(d))
struct Constant <: DetrendOperation end
struct Linear <: DetrendOperation end
Base.Integer(::Constant) = 1
Base.Integer(::Linear) = 2
const CONSTANT = Constant()
const LINEAR = Linear()

abstract type NoiseType <: Integer end
Base.Int32(d::NoiseType) = Int32(Integer(d))
struct Fifty <: NoiseType end
struct Sixty <: NoiseType end
Base.Integer(::Fifty) = 1
Base.Integer(::Sixty) = 2
const FIFTY = Fifty()
const SIXTY = Sixty()

@brainflow_rethrow function perform_lowpass(data, sampling_rate::Integer, cutoff::Float64, order::Integer,
    filter_type::Integer, ripple::Float64)
    ccall((:perform_lowpass, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Float64, Cint, Cint, Float64),
            data, length(data), Int32(sampling_rate), Float64(cutoff), Int32(order), Int32(filter_type), Float64(ripple))
    return
end

@brainflow_rethrow function perform_highpass(data, sampling_rate::Integer, cutoff::Float64, order::Integer,
    filter_type::Integer, ripple::Float64
)
    ccall((:perform_highpass, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Float64, Cint, Cint, Float64),
            data, length(data), Int32(sampling_rate), Float64(cutoff), Int32(order), Int32(filter_type), Float64(ripple))
    return
end

@brainflow_rethrow function perform_bandpass(data, sampling_rate::Integer, center_freq::Float64,
    band_width::Float64, order::Integer, filter_type::Integer, ripple::Float64)
    ccall((:perform_bandpass, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Float64, Float64, Cint, Cint, Float64),
            data, length(data), Int32(sampling_rate), Float64(center_freq), Float64(band_width), Int32(order), Int32(filter_type), Float64(ripple))
    return
end

@brainflow_rethrow function perform_bandstop(data, sampling_rate::Integer, center_freq::Float64,
    band_width::Float64, order::Integer, filter_type::Integer, ripple::Float64)
    ccall((:perform_bandstop, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Float64, Float64, Cint, Cint, Float64),
            data, length(data), Int32(sampling_rate), Float64(center_freq), Float64(band_width), Int32(order), Int32(filter_type), Float64(ripple))
    return
end

@brainflow_rethrow function remove_environmental_noise(data, sampling_rate::Integer, noise_type::Integer)
    ccall((:remove_environmental_noise, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Cint),
            data, length(data), Int32(sampling_rate), Int32(noise_type))
    return
end

@brainflow_rethrow function perform_rolling_filter(data, period::Integer, operation::Integer)
    ccall((:perform_rolling_filter, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Cint),
            data, length(data), Int32(period), Int32(operation))
    return
end

@brainflow_rethrow function detrend(data, operation)
    ccall((:detrend, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint),
            data, length(data), Int32(operation))
    return
end

@brainflow_rethrow function perform_wavelet_denoising(data, wavelet::String, decomposition_level::Integer)
    ccall((:perform_wavelet_denoising, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Ptr{UInt8}, Cint),
            data, length(data), wavelet, Int32(decomposition_level))
    return
end

@brainflow_rethrow function perform_downsampling(data, period::Integer, operation::Integer)
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

@brainflow_rethrow function perform_wavelet_transform(data, wavelet::String, decomposition_level::Integer)
    wavelet_coeffs = Vector{Float64}(undef, length(data) + 2 * (40 + 1))
    lengths = Vector{Cint}(undef, decomposition_level + 1)
    ccall((:perform_wavelet_transform, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Ptr{UInt8}, Cint, Ptr{Float64}, Ptr{Cint}),
            data, length(data), wavelet, Int32(decomposition_level), wavelet_coeffs, lengths)
    return wavelet_coeffs[1:sum(lengths)], lengths
end


@brainflow_rethrow function perform_inverse_wavelet_transform(wavelet_output, original_data_len::Integer, wavelet::String, decomposition_level::Integer)
    original_data = Vector{Float64}(undef, original_data_len)
    ccall((:perform_inverse_wavelet_transform, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Ptr{UInt8}, Cint, Ptr{Float64}, Ptr{Float64}),
            wavelet_output[1], Int32(original_data_len), wavelet, Int32(decomposition_level), wavelet_output[2], original_data)
    return original_data
end

@brainflow_rethrow function get_csp(data, labels)
    n_epochs = size(data, 3)
    n_channels = size(data, 1)
    n_times = size(data, 2)

    temp_data1d = Vector{Float64}(undef, Integer(n_epochs * n_channels * n_times))
    for e=1:n_epochs
        for c=1:n_channels
            for t=1:n_times
                temp_data1d[(e-1) * n_channels * n_times + (c-1) * n_times + t] = data[c, t, e]
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

@brainflow_rethrow function get_window(window_function::Integer, window_len::Integer)
    window_data = Vector{Float64}(undef, Integer(window_len))
    ccall((:get_window, DATA_HANDLER_INTERFACE), Cint, (Cint, Cint, Ptr{Float64}),
    Int32(window_function), Int32(window_len), window_data)
    return window_data
end

@brainflow_rethrow function perform_fft(data, window::Integer)

    function is_power_of_two(value)
        (value != 0) && (value & (value - 1) == 0)
    end

    if !is_power_of_two(length(data))
        throw(BrainFlowError(string("Data Len must be power of two ", INVALID_ARGUMENTS_ERROR), Integer(INVALID_ARGUMENTS_ERROR)))
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

@brainflow_rethrow function get_avg_band_powers(data, channels, sampling_rate::Integer, apply_filter::Bool)

    shape = size(data)
    data_1d = reshape(transpose(data[channels,:]), (1, size(channels)[1] * shape[2]))
    data_1d = copy(data_1d)

    temp_avgs = Vector{Float64}(undef, 5)
    temp_stddevs = Vector{Float64}(undef, 5)

    ccall((:get_avg_band_powers, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Cint, Cint, Ptr{Float64}, Ptr{Float64}),
            data_1d, size(channels)[1], shape[2], Int32(sampling_rate), Int32(apply_filter), temp_avgs, temp_stddevs)
    return temp_avgs, temp_stddevs
end

@brainflow_rethrow function get_psd(data, sampling_rate::Integer, window::Integer)

    function is_power_of_two(value)
        (value != 0) && (value & (value - 1) == 0)
    end

    if !is_power_of_two(length(data))
        throw(BrainFlowError(string("Data Len must be power of two ", INVALID_ARGUMENTS_ERROR), Integer(INVALID_ARGUMENTS_ERROR)))
    end

    temp_ampls = Vector{Float64}(undef, Integer(length(data) / 2) + 1)
    temp_freqs = Vector{Float64}(undef, Integer(length(data) / 2) + 1)

    ccall((:get_psd, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Cint, Cint, Cint, Ptr{Float64}, Ptr{Float64}),
            data, length(data), Int32(sampling_rate), Int32(window), temp_ampls, temp_freqs)
    return temp_ampls, temp_freqs
end

@brainflow_rethrow function get_psd_welch(data, nfft::Integer, overlap::Integer, sampling_rate::Integer, window::Integer)

    function is_power_of_two(value)
        (value != 0) && (value & (value - 1) == 0)
    end

    if !is_power_of_two(nfft)
        throw(BrainFlowError(string("nfft must be power of two ", INVALID_ARGUMENTS_ERROR), Integer(INVALID_ARGUMENTS_ERROR)))
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

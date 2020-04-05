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


function perform_lowpass(data, sampling_rate, cutoff, order, filter_type, ripple)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:perform_lowpass, "DataHandler.dll"), Cint, (Ptr{Float64}, Cint, Cint, Float64, Cint, Cint, Float64),
        	data, length(data), sampling_rate, cutoff, order, filter_type, ripple)
    elseif Sys.isapple()
        ec = ccall((:perform_lowpass, "libDataHandler.dylib"), Cint, (Ptr{Float64}, Cint, Cint, Float64, Cint, Cint, Float64),
        	data, length(data), sampling_rate, cutoff, order, filter_type, ripple)
    else
        ec = ccall((:perform_lowpass, "libDataHandler.so"), Cint, (Ptr{Float64}, Cint, Cint, Float64, Cint, Cint, Float64),
        	data, length(data), sampling_rate, cutoff, order, filter_type, ripple)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in perform_lowpass ", ec), ec))
    end
    return
end


function perform_highpass(data, sampling_rate, cutoff, order, filter_type, ripple)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:perform_highpass, "DataHandler.dll"), Cint, (Ptr{Float64}, Cint, Cint, Float64, Cint, Cint, Float64),
        	data, length(data), sampling_rate, cutoff, order, filter_type, ripple)
    elseif Sys.isapple()
        ec = ccall((:perform_highpass, "libDataHandler.dylib"), Cint, (Ptr{Float64}, Cint, Cint, Float64, Cint, Cint, Float64),
        	data, length(data), sampling_rate, cutoff, order, filter_type, ripple)
    else
        ec = ccall((:perform_highpass, "libDataHandler.so"), Cint, (Ptr{Float64}, Cint, Cint, Float64, Cint, Cint, Float64),
        	data, length(data), sampling_rate, cutoff, order, filter_type, ripple)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in perform_highpass ", ec), ec))
    end
    return
end


function perform_bandpass(data, sampling_rate, center_freq, band_width, order, filter_type, ripple)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:perform_bandpass, "DataHandler.dll"), Cint, (Ptr{Float64}, Cint, Cint, Float64, Float64, Cint, Cint, Float64),
        	data, length(data), sampling_rate, center_freq, band_width, order, filter_type, ripple)
    elseif Sys.isapple()
        ec = ccall((:perform_bandpass, "libDataHandler.dylib"), Cint, (Ptr{Float64}, Cint, Cint, Float64, Float64, Cint, Cint, Float64),
        	data, length(data), sampling_rate, center_freq, band_width, order, filter_type, ripple)
    else
        ec = ccall((:perform_bandpass, "libDataHandler.so"), Cint, (Ptr{Float64}, Cint, Cint, Float64, Float64, Cint, Cint, Float64),
        	data, length(data), sampling_rate, center_freq, band_width, order, filter_type, ripple)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in perform_bandpass ", ec), ec))
    end
    return
end


function perform_bandstop(data, sampling_rate, center_freq, band_width, order, filter_type, ripple)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:perform_bandstop, "DataHandler.dll"), Cint, (Ptr{Float64}, Cint, Cint, Float64, Float64, Cint, Cint, Float64),
        	data, length(data), sampling_rate, center_freq, band_width, order, filter_type, ripple)
    elseif Sys.isapple()
        ec = ccall((:perform_bandstop, "libDataHandler.dylib"), Cint, (Ptr{Float64}, Cint, Cint, Float64, Float64, Cint, Cint, Float64),
        	data, length(data), sampling_rate, center_freq, band_width, order, filter_type, ripple)
    else
        ec = ccall((:perform_bandstop, "libDataHandler.so"), Cint, (Ptr{Float64}, Cint, Cint, Float64, Float64, Cint, Cint, Float64),
        	data, length(data), sampling_rate, center_freq, band_width, order, filter_type, ripple)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in perform_bandstop ", ec), ec))
    end
    return
end


function perform_rolling_filter(data, period, operation)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:perform_rolling_filter, "DataHandler.dll"), Cint, (Ptr{Float64}, Cint, Cint, Cint),
        	data, length(data), period, operation)
    elseif Sys.isapple()
        ec = ccall((:perform_rolling_filter, "libDataHandler.dylib"), Cint, (Ptr{Float64}, Cint, Cint, Cint),
        	data, length(data), period, operation)
    else
        ec = ccall((:perform_rolling_filter, "libDataHandler.so"), Cint, (Ptr{Float64}, Cint, Cint, Cint),
        	data, length(data), period, operation)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in perform_rolling_filter ", ec), ec))
    end
    return
end


function perform_wavelet_denoising(data, wavelet, decomposition_level)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:perform_wavelet_denoising, "DataHandler.dll"), Cint, (Ptr{Float64}, Cint, Ptr{UInt8}, Cint),
        	data, length(data), wavelet, decomposition_level)
    elseif Sys.isapple()
        ec = ccall((:perform_wavelet_denoising, "libDataHandler.dylib"), Cint, (Ptr{Float64}, Cint, Ptr{UInt8}, Cint),
        	data, length(data), wavelet, decomposition_level)
    else
        ec = ccall((:perform_wavelet_denoising, "libDataHandler.so"), Cint, (Ptr{Float64}, Cint, Ptr{UInt8}, Cint),
        	data, length(data), wavelet, decomposition_level)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in perform_wavelet_denoising ", ec), ec))
    end
    return
end

function perform_downsampling(data, period, operation)
	len = Integer(floor(length(data) / period))
	downsampled_data = Vector{Float64}(undef, len)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:perform_downsampling, "DataHandler.dll"), Cint, (Ptr{Float64}, Cint, Cint, Cint, Ptr{Float64}),
        	data, length(data), period, operation, downsampled_data)
    elseif Sys.isapple()
        ec = ccall((:perform_downsampling, "libDataHandler.dylib"), Cint, (Ptr{Float64}, Cint, Cint, Cint, Ptr{Float64}),
        	data, length(data), period, operation, downsampled_data)
    else
        ec = ccall((:perform_downsampling, "libDataHandler.so"), Cint, (Ptr{Float64}, Cint, Cint, Cint, Ptr{Float64}),
        	data, length(data), period, operation, downsampled_data)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in perform_downsampling ", ec), ec))
    end
    downsampled_data
end


function write_file(data, file_name, file_mode)
	shape = size(data)
	flatten = reshape(transpose(data), (1, shape[1] * shape[2]))
	flatten = copy(flatten)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:write_file, "DataHandler.dll"), Cint, (Ptr{Float64}, Cint, Cint, Ptr{UInt8}, Ptr{UInt8}),
        	flatten, shape[1], shape[2], file_name, file_mode)
    elseif Sys.isapple()
        ec = ccall((:write_file, "libDataHandler.dylib"), Cint, (Ptr{Float64}, Cint, Cint, Ptr{UInt8}, Ptr{UInt8}),
        	flatten, shape[1], shape[2], file_name, file_mode)
    else
        ec = ccall((:write_file, "libDataHandler.so"), Cint, (Ptr{Float64}, Cint, Cint, Ptr{UInt8}, Ptr{UInt8}),
        	flatten, shape[1], shape[2], file_name, file_mode)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in write_file ", ec), ec))
    end
    return
end


function read_file(file_name)
	num_elements = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:get_num_elements_in_file, "DataHandler.dll"), Cint, (Ptr{UInt8}, Ptr{Cint}),
        		file_name, num_elements)
    elseif Sys.isapple()
        ec = ccall((:get_num_elements_in_file, "libDataHandler.dylib"), Cint, (Ptr{UInt8}, Ptr{Cint}),
        		file_name, num_elements)
    else
        ec = ccall((:get_num_elements_in_file, "libDataHandler.so"), Cint, (Ptr{UInt8}, Ptr{Cint}),
        		file_name, num_elements)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get_num_elements_in_file ", ec), ec))
    end
    
    data = Vector{Float64}(undef, num_elements[1])
    num_cols = Vector{Cint}(undef, 1)
    num_rows = Vector{Cint}(undef, 1)

    if Sys.iswindows()
        ec = ccall((:read_file, "DataHandler.dll"), Cint, (Ptr{Float64}, Ptr{Cint}, Ptr{Cint}, Ptr{UInt8}, Cint),
        		data, num_rows, num_cols, file_name, num_elements[1])
    elseif Sys.isapple()
        ec = ccall((:read_file, "libDataHandler.dylib"), Cint, (Ptr{Float64}, Ptr{Cint}, Ptr{Cint}, Ptr{UInt8}, Cint),
        		data, num_rows, num_cols, file_name, num_elements[1])
    else
        ec = ccall((:read_file, "libDataHandler.so"), Cint, (Ptr{Float64}, Ptr{Cint}, Ptr{Cint}, Ptr{UInt8}, Cint),
        		data, num_rows, num_cols, file_name, num_elements[1])
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in read_file ", ec), ec))
    end
    value = transpose(reshape(data, (num_cols[1], num_rows[1])))
    value
end


function perform_wavelet_transform(data, wavelet, decomposition_level)
	wavelet_coeffs = Vector{Float64}(undef, length(data) + 2 * (40 + 1))
	lengths = Vector{Cint}(undef, decomposition_level + 1)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:perform_wavelet_transform, "DataHandler.dll"), Cint, (Ptr{Float64}, Cint, Ptr{UInt8}, Cint, Ptr{Float64}, Ptr{Cint}),
        	data, length(data), wavelet, decomposition_level, wavelet_coeffs, lengths)
    elseif Sys.isapple()
        ec = ccall((:perform_wavelet_transform, "libDataHandler.dylib"), Cint, (Ptr{Float64}, Cint, Ptr{UInt8}, Cint, Ptr{Float64}, Ptr{Cint}),
        	data, length(data), wavelet, decomposition_level, wavelet_coeffs, lengths)
    else
        ec = ccall((:perform_wavelet_transform, "libDataHandler.so"), Cint, (Ptr{Float64}, Cint, Ptr{UInt8}, Cint, Ptr{Float64}, Ptr{Cint}),
        	data, length(data), wavelet, decomposition_level, wavelet_coeffs, lengths)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in perform_wavelet_transform ", ec), ec))
    end
    wavelet_coeffs[1:sum(lengths)], lengths
end


function perform_inverse_wavelet_transform(wavelet_output, original_data_len, wavelet, decomposition_level)
	original_data = Vector{Float64}(undef, original_data_len)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:perform_inverse_wavelet_transform, "DataHandler.dll"), Cint, (Ptr{Float64}, Cint, Ptr{UInt8}, Cint, Ptr{Float64}, Ptr{Float64}),
        	wavelet_output[1], original_data_len, wavelet, decomposition_level, wavelet_output[2], original_data)
    elseif Sys.isapple()
        ec = ccall((:perform_inverse_wavelet_transform, "libDataHandler.dylib"), Cint, (Ptr{Float64}, Cint, Ptr{UInt8}, Cint, Ptr{Float64}, Ptr{Float64}),
        	wavelet_output[1], original_data_len, wavelet, decomposition_level, wavelet_output[2], original_data)
    else
        ec = ccall((:perform_inverse_wavelet_transform, "libDataHandler.so"), Cint, (Ptr{Float64}, Cint, Ptr{UInt8}, Cint, Ptr{Float64}, Ptr{Float64}),
        	wavelet_output[1], original_data_len, wavelet, decomposition_level, wavelet_output[2], original_data)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in perform_inverse_wavelet_transform ", ec), ec))
    end
    original_data
end


function perform_fft(data)

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
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:perform_fft, "DataHandler.dll"), Cint, (Ptr{Float64}, Cint, Ptr{Float64}, Ptr{Float64}),
        	data, length(data), temp_re, temp_im)
    elseif Sys.isapple()
        ec = ccall((:perform_fft, "libDataHandler.dylib"), Cint, (Ptr{Float64}, Cint, Ptr{Float64}, Ptr{Float64}),
        	data, length(data), temp_re, temp_im)
    else
        ec = ccall((:perform_fft, "libDataHandler.so"), Cint, (Ptr{Float64}, Cint, Ptr{Float64}, Ptr{Float64}),
        	data, length(data), temp_re, temp_im)
    end
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
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:perform_ifft, "DataHandler.dll"), Cint, (Ptr{Float64}, Ptr{Float64}, Cint, Ptr{Float64}),
        	temp_re, temp_im, length(res), res)
    elseif Sys.isapple()
        ec = ccall((:perform_ifft, "libDataHandler.dylib"), Cint, (Ptr{Float64}, Ptr{Float64}, Cint, Ptr{Float64}),
        	temp_re, temp_im, length(res), res)
    else
        ec = ccall((:perform_ifft, "libDataHandler.so"), Cint, (Ptr{Float64}, Ptr{Float64}, Cint, Ptr{Float64}),
        	temp_re, temp_im, length(res), res)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in perform_ifft ", ec), ec))
    end
    res
end

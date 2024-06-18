classdef DataFilter
    % DataFilter class for signal processing
    methods(Static)

        function lib_name = load_lib()
            if ispc
                if not(libisloaded('DataHandler'))
                    loadlibrary('DataHandler.dll', 'data_handler.h');
                end
                lib_name = 'DataHandler';
            elseif ismac
                if not(libisloaded('libDataHandler'))
                    loadlibrary('libDataHandler.dylib', 'data_handler.h');
                end
                lib_name = 'libDataHandler';
            elseif isunix
                if not(libisloaded('libDataHandler'))
                    loadlibrary('libDataHandler.so', 'data_handler.h');
                end
                lib_name = 'libDataHandler';
            else
                error('OS not supported!')
            end
        end

        function check_ec(ec, task_name)
            if(ec ~= int32(BrainFlowExitCodes.STATUS_OK))
                error('Non zero ec: %d, for task: %s', ec, task_name)
            end
        end
        
        function version = get_version()
            % get version
            task_name = 'get_version_data_handler';
            lib_name = DataFilter.load_lib();
            % no way to understand how it works in matlab, used this link
            % https://nl.mathworks.com/matlabcentral/answers/131446-what-data-type-do-i-need-to-calllib-with-pointer-argument-char%
            [exit_code, version] = calllib(lib_name, task_name, blanks(64), 64, 64);
            DataFilter.check_ec(exit_code, task_name);
        end
        
        function log_message(log_level, message)
            % write message to Data Filter logger
            task_name = 'log_message_data_handler';
            lib_name = DataFilter.load_lib();
            exit_code = calllib(lib_name, task_name, log_level, message);
            DataFilter.check_ec(exit_code, task_name);
        end

        function set_log_level(log_level)
            % set log level for DataFilter
            task_name = 'set_log_level_data_handler';
            lib_name = DataFilter.load_lib();
            exit_code = calllib(lib_name, task_name, log_level);
            DataFilter.check_ec(exit_code, task_name);
        end

        function set_log_file(log_file)
            % set log file for DataFilter
            task_name = 'set_log_file_data_handler';
            lib_name = DataFilter.load_lib();
            exit_code = calllib(lib_name, task_name, log_file);
            DataFilter.check_ec(exit_code, task_name);
        end

        function enable_data_logger()
            % enable logger with level INFO
            DataFilter.set_log_level(int32(2))
        end

        function enable_dev_data_logger()
            % enable logger with level TRACE
            DataFilter.set_log_level(int32(0))
        end

        function disable_data_logger()
            % disable logger
            DataFilter.set_log_level(int32(6))
        end

        function filtered_data = perform_lowpass(data, sampling_rate, cutoff, order, filter_type, ripple)
            % perform lowpass filtering
            task_name = 'perform_lowpass';
            temp = libpointer('doublePtr', data);
            lib_name = DataFilter.load_lib();
            exit_code = calllib(lib_name, task_name, temp, size(data, 2), sampling_rate, cutoff, order, int32(filter_type), ripple);
            DataFilter.check_ec(exit_code, task_name);
            filtered_data = temp.Value;
        end
        
        function filtered_data = perform_highpass(data, sampling_rate, cutoff, order, filter_type, ripple)
            % perform highpass filtering
            task_name = 'perform_highpass';
            temp = libpointer('doublePtr', data);
            lib_name = DataFilter.load_lib();
            exit_code = calllib(lib_name, task_name, temp, size(data, 2), sampling_rate, cutoff, order, int32(filter_type), ripple);
            DataFilter.check_ec(exit_code, task_name);
            filtered_data = temp.Value;
        end
        
        function filtered_data = perform_bandpass(data, sampling_rate, start_freq, stop_freq, order, filter_type, ripple)
            % perform bandpass filtering
            task_name = 'perform_bandpass';
            temp = libpointer('doublePtr', data);
            lib_name = DataFilter.load_lib();
            exit_code = calllib(lib_name, task_name, temp, size(data, 2), sampling_rate, start_freq, stop_freq, order, int32(filter_type), ripple);
            DataFilter.check_ec(exit_code, task_name);
            filtered_data = temp.Value;
        end
        
        function filtered_data = perform_bandstop(data, sampling_rate, start_freq, stop_freq, order, filter_type, ripple)
            % perform bandpass filtering
            task_name = 'perform_bandstop';
            temp = libpointer('doublePtr', data);
            lib_name = DataFilter.load_lib();
            exit_code = calllib(lib_name, task_name, temp, size(data, 2), sampling_rate, start_freq, stop_freq, order, int32(filter_type), ripple);
            DataFilter.check_ec(exit_code, task_name);
            filtered_data = temp.Value;
        end
        
        function filtered_data = remove_environmental_noise(data, sampling_rate, noise_type)
            % perform noth filtering
            task_name = 'remove_environmental_noise';
            temp = libpointer('doublePtr', data);
            lib_name = DataFilter.load_lib();
            exit_code = calllib(lib_name, task_name, temp, size(data, 2), sampling_rate, int32(noise_type));
            DataFilter.check_ec(exit_code, task_name);
            filtered_data = temp.Value;
        end

        function filtered_data = perform_rolling_filter(data, period, operation)
            % apply rolling filter
            task_name = 'perform_rolling_filter';
            temp = libpointer('doublePtr', data);
            lib_name = DataFilter.load_lib();
            exit_code = calllib(lib_name, task_name, temp, size(data, 2), period, int32(operation));
            DataFilter.check_ec(exit_code, task_name);
            filtered_data = temp.Value;
        end
        
        function new_data = detrend(data, operation)
            % remove trend from data
            task_name = 'detrend';
            temp = libpointer('doublePtr', data);
            lib_name = DataFilter.load_lib();
            exit_code = calllib(lib_name, task_name, temp, size(data, 2), int32(operation));
            DataFilter.check_ec(exit_code, task_name);
            new_data = temp.Value;
        end
        
        function downsampled_data = perform_downsampling(data, period, operation)
            % downsample data
            task_name = 'perform_downsampling';
            temp_input = libpointer('doublePtr', data);
            lib_name = DataFilter.load_lib();
            temp_output = libpointer('doublePtr', zeros(1, int32(size(data,2) / period)));
            exit_code = calllib(lib_name, task_name, temp_input, size(data, 2), period, int32(operation), temp_output);
            DataFilter.check_ec(exit_code, task_name);
            downsampled_data = temp_output.Value;
        end
        
        function restored_data = restore_data_from_wavelet_detailed_coeffs(data, wavelet, decomposition_level, level_to_restore)
            % restore data from a single wavelet level
            task_name = 'restore_data_from_wavelet_detailed_coeffs';
            temp_input = libpointer('doublePtr', data);
            lib_name = DataFilter.load_lib();
            temp_output = libpointer('doublePtr', zeros(1, int32(size(data,2))));
            exit_code = calllib(lib_name, task_name, temp_input, size(data, 2), int32(wavelet), decomposition_level, level_to_restore, temp_output);
            DataFilter.check_ec(exit_code, task_name);
            restored_data = temp_output.Value;
        end
        
        function peaks = detect_peaks_z_score(data, lag, threshold, influence)
            % peaks detection using z score algorithm
            task_name = 'detect_peaks_z_score';
            temp_input = libpointer('doublePtr', data);
            lib_name = DataFilter.load_lib();
            temp_output = libpointer('doublePtr', zeros(1, int32(size(data,2))));
            exit_code = calllib(lib_name, task_name, temp_input, size(data, 2), int32(lag), threshold, influence, temp_output);
            DataFilter.check_ec(exit_code, task_name);
            peaks = temp_output.Value;
        end
        
        function [wavelet_data, wavelet_sizes] = perform_wavelet_transform(data, wavelet, decomposition_level, extension)
            % perform wavelet transform
            task_name = 'perform_wavelet_transform';
            temp_input = libpointer('doublePtr', data);
            lib_name = DataFilter.load_lib();
            temp_output = libpointer('doublePtr', zeros(1, int32(size(data, 2) + 2 *(40 + 1))));
            lenghts = libpointer('int32Ptr', zeros(1, decomposition_level + 1));
            exit_code = calllib(lib_name, task_name, temp_input, size(data, 2), wavelet, decomposition_level, extension, temp_output, lenghts);
            DataFilter.check_ec(exit_code, task_name);
            wavelet_data = temp_output.Value(1,1: sum(lenghts.Value));
            wavelet_sizes = lenghts.Value;
        end
        
        function original_data = perform_inverse_wavelet_transform(wavelet_data, wavelet_sizes, original_data_len, wavelet, decomposition_level, extension)
            % perform inverse wavelet transform
            task_name = 'perform_inverse_wavelet_transform';
            lib_name = DataFilter.load_lib();
            wavelet_data_ptr = libpointer('doublePtr', wavelet_data);
            wavelet_sizes_ptr = libpointer('int32Ptr', wavelet_sizes);
            output_ptr = libpointer('doublePtr', zeros(1, original_data_len));
            exit_code = calllib(lib_name, task_name, wavelet_data_ptr, original_data_len, wavelet, decomposition_level, extension, wavelet_sizes_ptr, output_ptr);
            DataFilter.check_ec(exit_code, task_name);
            original_data = output_ptr.Value;
        end
        
        function denoised_data = perform_wavelet_denoising(data, wavelet, decomposition_level, denoising, threshold, extention, noise_level)
            % perform wavelet denoising
            task_name = 'perform_wavelet_denoising';
            temp = libpointer('doublePtr', data);
            lib_name = DataFilter.load_lib();
            exit_code = calllib(lib_name, task_name, temp, size(data, 2), wavelet, decomposition_level, denoising, threshold, extention, noise_level);
            DataFilter.check_ec(exit_code, task_name);
            denoised_data = temp.Value;
        end
        
        function [filters, eigenvalues] = get_csp(data, labels)
            % get common spatial patterns
            task_name = 'get_csp';
            n_epochs = size(data, 1);
            n_channels = size(data, 2);
            n_times = size(data, 3);
            lib_name = DataFilter.load_lib();
            data1d = zeros(1, n_epochs * n_channels * n_times);
            for e=1:n_epochs
                for c=1:n_channels
                    for t=1:n_times
                        idx = (e-1) * n_channels * n_times + (c-1) * n_times + t;
                        data1d(idx) = data(e,c,t);
                    end
                end
            end
            data1d_ptr = libpointer('doublePtr', data1d);
            labels_ptr = libpointer('doublePtr', labels);
            output_filters_ptr = libpointer('doublePtr', zeros(1, n_channels * n_channels));
            output_eigenvalues_ptr = libpointer('doublePtr', zeros(1, int32(n_channels)));
            exit_code = calllib(lib_name, task_name, data1d_ptr, labels_ptr, n_epochs, n_channels, n_times, output_filters_ptr, output_eigenvalues_ptr);
            DataFilter.check_ec(exit_code, task_name);
            filters = zeros(n_channels, n_channels);
            for i=1:n_channels
                for j=1:n_channels
                    filters(i, j) = output_filters_ptr.Value((i-1) * n_channels + j);
                end 
            end
            eigenvalues = output_eigenvalues_ptr.Value;
        end

        function window_data = get_window(window_function, window_len)
            % get window
            task_name = 'get_window';
            lib_name = DataFilter.load_lib();
            temp_output = libpointer('doublePtr', zeros(1, int32(window_len)));
            exit_code = calllib(lib_name, task_name, window_function, window_len, temp_output);
            DataFilter.check_ec(exit_code, task_name);
            window_data = temp_output.Value;
        end

        function stddev = calc_stddev(data)
            % calc stddev
            task_name = 'calc_stddev';
            temp_input = libpointer('doublePtr', data);
            output = libpointer('doublePtr', 0);
            lib_name = DataFilter.load_lib();
            exit_code = calllib(lib_name, task_name, temp_input, 0, size(data, 2), output);
            DataFilter.check_ec(exit_code, task_name);
            stddev = output.Value;
        end
        
        function railed = get_railed_percentage(data, gain)
            % calc railed percentage
            task_name = 'get_railed_percentage';
            temp_input = libpointer('doublePtr', data);
            output = libpointer('doublePtr', 0);
            lib_name = DataFilter.load_lib();
            exit_code = calllib(lib_name, task_name, temp_input, size(data, 2), gain, output);
            DataFilter.check_ec(exit_code, task_name);
            railed = output.Value;
        end

        function spo2 = get_oxygen_level(ppg_ir, ppg_red, sampling_rate, coef1, coef2, coef3)
            % calc oxygen level
            task_name = 'get_oxygen_level';
            temp_input_ir = libpointer('doublePtr', ppg_ir);
            temp_input_red = libpointer('doublePtr', ppg_red);
            output = libpointer('doublePtr', 0);
            lib_name = DataFilter.load_lib();
            exit_code = calllib(lib_name, task_name, temp_input_ir, temp_input_red, size(ppg_ir, 2), sampling_rate, coef1, coef2, coef3, output);
            DataFilter.check_ec(exit_code, task_name);
            spo2 = output.Value;
        end

        function rate = get_heart_rate(ppg_ir, ppg_red, sampling_rate, fft_size)
            % calc heart rate
            task_name = 'get_heart_rate';
            temp_input_ir = libpointer('doublePtr', ppg_ir);
            temp_input_red = libpointer('doublePtr', ppg_red);
            output = libpointer('doublePtr', 0);
            lib_name = DataFilter.load_lib();
            exit_code = calllib(lib_name, task_name, temp_input_ir, temp_input_red, size(ppg_ir, 2), sampling_rate, fft_size, output);
            DataFilter.check_ec(exit_code, task_name);
            rate = output.Value;
        end
        
        function fft_data = perform_fft(data, window)
            % perform fft
            task_name = 'perform_fft';
            n = size(data, 2);
            temp_input = libpointer('doublePtr', data);
            lib_name = DataFilter.load_lib();
            temp_re = libpointer('doublePtr', zeros(1, int32(n / 2 + 1)));
            temp_im = libpointer('doublePtr', zeros(1, int32(n / 2 + 1)));
            exit_code = calllib(lib_name, task_name, temp_input, n, window, temp_re, temp_im);
            DataFilter.check_ec(exit_code, task_name);
            fft_data = complex(temp_re.Value, temp_im.Value);
        end

        function data = perform_ifft(fft_data)
            % perform inverse fft
            task_name = 'perform_ifft';
            real_data = real(fft_data);
            imag_data = imag(fft_data);
            real_input = libpointer('doublePtr', real_data);
            imag_input = libpointer('doublePtr', imag_data);
            output_len = 2 *(size(fft_data, 2) - 1);
            output = libpointer('doublePtr', zeros(1, output_len));
            lib_name = DataFilter.load_lib();
            exit_code = calllib(lib_name, task_name, real_input, imag_input, output_len, output);
            DataFilter.check_ec(exit_code, task_name);
            data = output.Value;
        end
        
        function [avg_bands, stddev_bands] = get_avg_band_powers(data, channels, sampling_rate, apply_filters)
            start_freqs = [2.0, 4.0, 8.0, 13.0, 30.0];
            stop_freqs = [4.0, 8.0, 13.0, 30.0, 45.0];
            [avg_bands, stddev_bands] = DataFilter.get_custom_band_powers(data, start_freqs, stop_freqs, channels, sampling_rate, apply_filters);
        end
            
        function [avg_bands, stddev_bands] = get_custom_band_powers(data, start_freqs, stop_freqs, channels, sampling_rate, apply_filters)
            % calculate average band powers
            task_name = 'get_custom_band_powers';
            data_1d = data(channels, :);
            data_1d = transpose(data_1d);
            data_1d = data_1d(:);
            temp_input = libpointer('doublePtr', data_1d);
            lib_name = DataFilter.load_lib();
            temp_avgs = libpointer('doublePtr', zeros(1, size(start_freqs, 2)));
            temp_stddevs = libpointer('doublePtr', zeros(1, size(start_freqs, 2)));
            temp_start = libpointer('doublePtr', start_freqs); 
            temp_stop = libpointer('doublePtr', stop_freqs);
            exit_code = calllib(lib_name, task_name, temp_input, size(channels, 2), size(data,2), temp_start, temp_stop, size(start_freqs, 2), sampling_rate, int32(apply_filters), temp_avgs, temp_stddevs);
            DataFilter.check_ec(exit_code, task_name);
            avg_bands = temp_avgs.Value;
            stddev_bands = temp_stddevs.Value;
        end
        
        function [w_mat, k_mat, a_mat, s_mat] = perform_ica_select_channels(data, num_components, channels)
            % calculate ica
            task_name = 'perform_ica';
            data_1d = data(channels, :);
            data_1d = transpose(data_1d);
            data_1d = data_1d(:);
            temp_input = libpointer('doublePtr', data_1d);
            lib_name = DataFilter.load_lib();
            temp_w = libpointer('doublePtr', zeros(1, num_components * num_components));
            temp_k = libpointer('doublePtr', zeros(1, size(channels, 2) * num_components));
            temp_a = libpointer('doublePtr', zeros(1, num_components * size(channels, 2)));
            temp_s = libpointer('doublePtr', zeros(1, size(data, 2) * num_components));
            exit_code = calllib(lib_name, task_name, temp_input, size(channels, 2), size(data, 2), num_components, temp_w, temp_k, temp_a, temp_s);
            DataFilter.check_ec(exit_code, task_name);
            w_mat = transpose(reshape(temp_w.Value, [num_components, num_components]));
            k_mat = transpose(reshape(temp_k.Value, [size(channels,2), num_components]));
            a_mat = transpose(reshape(temp_a.Value, [num_components, size(channels, 2)]));
            s_mat = transpose(reshape(temp_s.Value, [size(data, 2), num_components]));
        end
        
        function [w_mat, k_mat, a_mat, s_mat] = perform_ica(data, num_components)
            % calculate ica
            channels = uint32(1):uint32(size(data,1));
            [w_mat, k_mat, a_mat, s_mat] = DataFilter.perform_ica_select_channels(data, num_components, channels);
        end
        
        function [ampls, freqs] = get_psd(data, sampling_rate, window)
            % calculate PSD
            task_name = 'get_psd';
            n = size(data, 2);
            temp_input = libpointer('doublePtr', data);
            lib_name = DataFilter.load_lib();
            temp_ampls = libpointer('doublePtr', zeros(1, int32(n / 2 + 1)));
            temp_freqs = libpointer('doublePtr', zeros(1, int32(n / 2 + 1)));
            exit_code = calllib(lib_name, task_name, temp_input, n, sampling_rate, window, temp_ampls, temp_freqs);
            DataFilter.check_ec(exit_code, task_name);
            ampls = temp_ampls.Value;
            freqs = temp_freqs.Value;
        end
        
        function [ampls, freqs] = get_psd_welch(data, nfft, overlap, sampling_rate, window)
            % calculate PSD using welch method
            task_name = 'get_psd_welch';
            temp_input = libpointer('doublePtr', data);
            lib_name = DataFilter.load_lib();
            temp_ampls = libpointer('doublePtr', zeros(1, int32(nfft / 2 + 1)));
            temp_freqs = libpointer('doublePtr', zeros(1, int32(nfft / 2 + 1)));
            exit_code = calllib(lib_name, task_name, temp_input, size(data, 2), nfft, overlap, sampling_rate, window, temp_ampls, temp_freqs);
            DataFilter.check_ec(exit_code, task_name);
            ampls = temp_ampls.Value;
            freqs = temp_freqs.Value;
        end
        
        function band_power = get_band_power(ampls, freqs, freq_start, freq_end)
            % calculate band power
            task_name = 'get_band_power';
            temp_input_ampl = libpointer('doublePtr', ampls);
            temp_input_freq = libpointer('doublePtr', freqs);
            lib_name = DataFilter.load_lib();
            temp_band = libpointer('doublePtr', 0);
            exit_code = calllib(lib_name, task_name, temp_input_ampl, temp_input_freq, size(ampls, 2), freq_start, freq_end, temp_band);
            DataFilter.check_ec(exit_code, task_name);
            band_power = temp_band.Value;
        end

        function output = get_nearest_power_of_two(value)
            % get nearest power of two
            lib_name = DataFilter.load_lib();
            task_name = 'get_nearest_power_of_two';
            power_of_two = libpointer('int32Ptr', 0);
            exit_code = calllib(lib_name, task_name, value, power_of_two);
            DataFilter.check_ec(exit_code, task_name);
            output = power_of_two.Value;
        end

        function write_file(data, file_name, file_mode)
            % write data to file, in file data will be transposed
            task_name = 'write_file';
            lib_name = DataFilter.load_lib();
            % convert to 1d array 1xnum_datapoints %
            transposed = transpose(data);
            flatten = transpose(transposed(:));
            temp = libpointer('doublePtr', flatten);
            exit_code = calllib(lib_name, task_name, temp, size(data,1), size(data, 2), file_name, file_mode);
            DataFilter.check_ec(exit_code, task_name);
        end
        
        function data = read_file(file_name)
            % read data from file
            lib_name = DataFilter.load_lib();

            task_name = 'get_num_elements_in_file';
            data_count = libpointer('int32Ptr', 0);
            exit_code = calllib(lib_name, task_name, file_name, data_count);
            DataFilter.check_ec(exit_code, task_name);
            
            num_rows = libpointer('int32Ptr', 0);
            num_cols = libpointer('int32Ptr', 0);
            data_array = libpointer('doublePtr', zeros(1, data_count.Value));
            task_name = 'read_file';
            exit_code = calllib(lib_name, task_name, data_array, num_rows, num_cols, file_name, data_count.Value);
            DataFilter.check_ec(exit_code, task_name);
            data =  transpose(reshape(data_array.Value(1, 1:data_count.Value), [num_cols.Value, num_rows.value]));
        end
        
    end
    
end
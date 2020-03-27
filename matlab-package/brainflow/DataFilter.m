classdef DataFilter
    
    methods (Static)

        function lib_name = load_lib ()
            if ispc
                if not (libisloaded ('DataHandler'))
                    loadlibrary ('DataHandler.dll', 'data_handler.h');
                end
                lib_name = 'DataHandler';
            elseif ismac
                if not (libisloaded ('libDataHandler'))
                    loadlibrary ('libDataHandler.dylib', 'data_handler.h');
                end
                lib_name = 'libDataHandler';
            elseif isunix
                if not (libisloaded ('libDataHandler'))
                    loadlibrary ('libDataHandler.so', 'data_handler.h');
                end
                lib_name = 'libDataHandler';
            else
                error ('OS not supported!')
            end
        end

        function check_ec (ec, task_name)
            if (ec ~= int32 (ExitCodes.STATUS_OK))
                error ('Non zero ec: %d, for task: %s', ec, task_name)
            end
        end

        function filtered_data = perform_lowpass (data, sampling_rate, cutoff, order, filter_type, ripple)
            task_name = 'perform_lowpass';
            temp = libpointer ('doublePtr', data);
            lib_name = DataFilter.load_lib ();
            exit_code = calllib (lib_name, task_name, temp, size (data, 2), sampling_rate, cutoff, order, int32 (filter_type), ripple);
            DataFilter.check_ec (exit_code, task_name);
            filtered_data = temp.Value;
        end
        
        function filtered_data = perform_highpass (data, sampling_rate, cutoff, order, filter_type, ripple)
            task_name = 'perform_highpass';
            temp = libpointer ('doublePtr', data);
            lib_name = DataFilter.load_lib ();
            exit_code = calllib (lib_name, task_name, temp, size (data, 2), sampling_rate, cutoff, order, int32 (filter_type), ripple);
            DataFilter.check_ec (exit_code, task_name);
            filtered_data = temp.Value;
        end
        
        function filtered_data = perform_bandpass (data, sampling_rate, center_freq, band_width, order, filter_type, ripple)
            task_name = 'perform_bandpass';
            temp = libpointer ('doublePtr', data);
            lib_name = DataFilter.load_lib ();
            exit_code = calllib (lib_name, task_name, temp, size (data, 2), sampling_rate, center_freq, band_width, order, int32 (filter_type), ripple);
            DataFilter.check_ec (exit_code, task_name);
            filtered_data = temp.Value;
        end
        
        function filtered_data = perform_bandstop (data, sampling_rate, center_freq, band_width, order, filter_type, ripple)
            task_name = 'perform_bandstop';
            temp = libpointer ('doublePtr', data);
            lib_name = DataFilter.load_lib ();
            exit_code = calllib (lib_name, task_name, temp, size (data, 2), sampling_rate, center_freq, band_width, order, int32 (filter_type), ripple);
            DataFilter.check_ec (exit_code, task_name);
            filtered_data = temp.Value;
        end

        function filtered_data = perform_rolling_filter (data, period, operation)
            task_name = 'perform_rolling_filter';
            temp = libpointer ('doublePtr', data);
            lib_name = DataFilter.load_lib ();
            exit_code = calllib (lib_name, task_name, temp, size (data, 2), period, int32 (operation));
            DataFilter.check_ec (exit_code, task_name);
            filtered_data = temp.Value;
        end
        
        function downsampled_data = perform_downsampling (data, period, operation)
            task_name = 'perform_downsampling';
            temp_input = libpointer ('doublePtr', data);
            lib_name = DataFilter.load_lib ();
            temp_output = libpointer ('doublePtr', zeros (1, int32 (size (data,2) / period)));
            exit_code = calllib (lib_name, task_name, temp_input, size (data, 2), period, int32 (operation), temp_output);
            DataFilter.check_ec (exit_code, task_name);
            downsampled_data = temp_output.Value;
        end
        
        function [wavelet_data, wavelet_sizes] = perform_wavelet_transform (data, wavelet, decomposition_level)
            task_name = 'perform_wavelet_transform';
            temp_input = libpointer ('doublePtr', data);
            lib_name = DataFilter.load_lib ();
            temp_output = libpointer ('doublePtr', zeros (1, int32 (size (data, 2) + 2 * (40 + 1))));
            lenghts = libpointer ('int32Ptr', zeros (1, decomposition_level + 1));
            exit_code = calllib (lib_name, task_name, temp_input, size (data, 2), wavelet, decomposition_level, temp_output, lenghts);
            DataFilter.check_ec (exit_code, task_name);
            wavelet_data = temp_output.Value(1,1: sum (lenghts.Value));
            wavelet_sizes = lenghts.Value;
        end
        
        function original_data = perform_inverse_wavelet_transform (wavelet_data, wavelet_sizes, original_data_len, wavelet, decomposition_level)
            task_name = 'perform_inverse_wavelet_transform';
            lib_name = DataFilter.load_lib ();
            wavelet_data_ptr = libpointer ('doublePtr', wavelet_data);
            wavelet_sizes_ptr = libpointer ('int32Ptr', wavelet_sizes);
            output_ptr = libpointer ('doublePtr', zeros (1, original_data_len));
            exit_code = calllib (lib_name, task_name, wavelet_data_ptr, original_data_len, wavelet, decomposition_level, wavelet_sizes_ptr, output_ptr);
            DataFilter.check_ec (exit_code, task_name);
            original_data = output_ptr.Value;
        end
        
        function denoised_data = perform_wavelet_denoising (data, wavelet, decomposition_level)
            task_name = 'perform_wavelet_denoising';
            temp = libpointer ('doublePtr', data);
            lib_name = DataFilter.load_lib ();
            exit_code = calllib (lib_name, task_name, temp, size (data, 2), wavelet, decomposition_level);
            DataFilter.check_ec (exit_code, task_name);
            denoised_data = temp.Value;
        end
        
        function fft_data = perform_fft (data)
            task_name = 'perform_fft';
            n = size (data, 2);
            if (bitand (n, n - 1) ~= 0)
                error ('For FFT shape must be power of 2!');
            end
            temp_input = libpointer ('doublePtr', data);
            lib_name = DataFilter.load_lib ();
            temp_re = libpointer ('doublePtr', zeros (1, int32 (n / 2 + 1)));
            temp_im = libpointer ('doublePtr', zeros (1, int32 (n / 2 + 1)));
            exit_code = calllib (lib_name, task_name, temp_input, n, temp_re, temp_im);
            DataFilter.check_ec (exit_code, task_name);
            fft_data = complex (temp_re.Value, temp_im.Value);
        end
        
        function data = perform_ifft (fft_data)
            task_name = 'perform_ifft';
            real_data = real (fft_data);
            imag_data = imag (fft_data);
            real_input = libpointer ('doublePtr', real_data);
            imag_input = libpointer ('doublePtr', imag_data);
            output_len = 2 * (size (fft_data, 2) - 1);
            output = libpointer ('doublePtr', zeros (1, output_len));
            lib_name = DataFilter.load_lib ();
            exit_code = calllib (lib_name, task_name, real_input, imag_input, output_len, output);
            DataFilter.check_ec (exit_code, task_name);
            data = output.Value;
        end
        
        function write_file (data, file_name, file_mode)
            task_name = 'write_file';
            lib_name = DataFilter.load_lib ();
            % convert to 1d array 1xnum_datapoints %
            transposed = transpose (data);
            flatten = transpose (transposed (:));
            temp = libpointer ('doublePtr', flatten);
            exit_code = calllib (lib_name, task_name, temp, size (data,1), size (data, 2), file_name, file_mode);
            DataFilter.check_ec (exit_code, task_name);
        end
        
        function data = read_file (file_name)
            lib_name = DataFilter.load_lib ();

            task_name = 'get_num_elements_in_file';
            data_count = libpointer ('int32Ptr', 0);
            exit_code = calllib (lib_name, task_name, file_name, data_count);
            DataFilter.check_ec (exit_code, task_name);
            
            num_rows = libpointer ('int32Ptr', 0);
            num_cols = libpointer ('int32Ptr', 0);
            data_array = libpointer ('doublePtr', zeros (1, data_count.Value));
            task_name = 'read_file';
            exit_code = calllib (lib_name, task_name, data_array, num_rows, num_cols, file_name, data_count.Value);
            DataFilter.check_ec (exit_code, task_name);
            data =  transpose (reshape (data_array.Value(1, 1:data_count.Value), [num_cols.Value, num_rows.value]));
        end
        
    end
    
end
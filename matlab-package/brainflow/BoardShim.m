classdef BoardShim
    % BoardShim object to communicate with device
    properties
        board_id
        master_board_id
        input_params_json
    end

    methods(Static)
        
        function lib_name = load_lib()
            if ispc
                if not(libisloaded('BoardController'))
                    loadlibrary('BoardController.dll', 'board_controller.h', 'addheader', 'board_info_getter');
                end
                lib_name = 'BoardController';
            elseif ismac
                if not(libisloaded('libBoardController'))
                    loadlibrary('libBoardController.dylib', 'board_controller.h', 'addheader', 'board_info_getter');
                end
                lib_name = 'libBoardController';
            elseif isunix
                if not(libisloaded('libBoardController'))
                    loadlibrary('libBoardController.so', 'board_controller.h', 'addheader', 'board_info_getter');
                end
                lib_name = 'libBoardController';
            else
                error('OS not supported!')
            end
        end
        
        function check_ec(ec, task_name)
            if (ec ~= int32(ExitCodes.STATUS_OK))
                error('Non zero ec: %d, for task: %s', ec, task_name)
            end
        end

        function set_log_level(log_level)
            % set log level for BoardShim
            task_name = 'set_log_level_board_controller';
            lib_name = BoardShim.load_lib();
            exit_code = calllib(lib_name, task_name, log_level);
            BoardShim.check_ec(exit_code, task_name);
        end

        function set_log_file(log_file)
            % set log file for BoardShim, logger uses stderr by default
            task_name = 'set_log_file_board_controller';
            lib_name = BoardShim.load_lib();
            exit_code = calllib(lib_name, task_name, log_file);
            BoardShim.check_ec(exit_code, task_name);
        end

        function enable_board_logger()
            % enable logger with level INFO
            BoardShim.set_log_level(int32(2))
        end

        function enable_dev_board_logger()
            % enable logger with level TRACE
            BoardShim.set_log_level(int32(0))
        end

        function disable_board_logger()
            % disable logger
            BoardShim.set_log_level(int32(6))
        end
        
        function log_message(log_level, message)
            % write message to BoardShim logger
            task_name = 'log_message_board_controller';
            lib_name = BoardShim.load_lib();
            exit_code = calllib(lib_name, task_name, log_level, message);
            BoardShim.check_ec(exit_code, task_name);
        end
        
        function sampling_rate = get_sampling_rate(board_id)
            % get sampling rate for provided board id
            task_name = 'get_sampling_rate';
            lib_name = BoardShim.load_lib();
            res = libpointer('int32Ptr', 0);
            exit_code = calllib(lib_name, task_name, board_id, res);
            BoardShim.check_ec(exit_code, task_name);
            sampling_rate = res.value;
        end
        
        function package_num_channel = get_package_num_channel(board_id)
            % get package num channel for provided board id
            task_name = 'get_package_num_channel';
            lib_name = BoardShim.load_lib();
            res = libpointer('int32Ptr', 0);
            exit_code = calllib(lib_name, task_name, board_id, res);
            BoardShim.check_ec(exit_code, task_name);
            % Matlab counts array elements from 1 while BrainFlow Core counts from 0%
            package_num_channel = res.value + 1;
        end
        
        function marker_channel = get_marker_channel(board_id)
            % get marker channel for provided board id
            task_name = 'get_marker_channel';
            lib_name = BoardShim.load_lib();
            res = libpointer('int32Ptr', 0);
            exit_code = calllib(lib_name, task_name, board_id, res);
            BoardShim.check_ec(exit_code, task_name);
            % Matlab counts array elements from 1 while BrainFlow Core counts from 0%
            marker_channel = res.value + 1;
        end

        function battery_channel = get_battery_channel(board_id)
            % get battery channel for provided board id
            task_name = 'get_battery_channel';
            res = libpointer('int32Ptr', 0);
            exit_code = calllib(lib_name, task_name, board_id, res);
            BoardShim.check_ec(exit_code, task_name);
            battery_channel = res.value + 1;
        end
        
        function num_rows = get_num_rows(board_id)
            % get num rows for provided board id
            task_name = 'get_num_rows';
            lib_name = BoardShim.load_lib();
            res = libpointer('int32Ptr', 0);
            exit_code = calllib(lib_name, task_name, board_id, res);
            BoardShim.check_ec(exit_code, task_name);
            num_rows = res.value;
        end
        
        function timestamp_channel = get_timestamp_channel(board_id)
            % get timestamp channel for provided board id
            task_name = 'get_timestamp_channel';
            lib_name = BoardShim.load_lib();
            res = libpointer('int32Ptr', 0);
            exit_code = calllib(lib_name, task_name, board_id, res);
            BoardShim.check_ec(exit_code, task_name);
            timestamp_channel = res.value + 1;
        end
        
        function board_descr = get_board_descr(board_id)
            % get board descr for provided board id
            task_name = 'get_board_descr';
            lib_name = BoardShim.load_lib();
            % no way to understand how it works in matlab, used this link
            % https://nl.mathworks.com/matlabcentral/answers/131446-what-data-type-do-i-need-to-calllib-with-pointer-argument-char%
            [exit_code, board_descr] = calllib(lib_name, task_name, board_id, blanks(16000), 16000);
            BoardShim.check_ec(exit_code, task_name);
            board_descr = jsondecode(board_descr);
        end
        
        function eeg_names = get_eeg_names(board_id)
            % get eeg names for provided board id
            task_name = 'get_eeg_names';
            lib_name = BoardShim.load_lib();
            % no way to understand how it works in matlab, used this link
            % https://nl.mathworks.com/matlabcentral/answers/131446-what-data-type-do-i-need-to-calllib-with-pointer-argument-char%
            [exit_code, eeg_names] = calllib(lib_name, task_name, board_id, blanks(4096), 4096);
            BoardShim.check_ec(exit_code, task_name);
            eeg_names = split(eeg_names, ',');
        end
        
        function device_name = get_device_name(board_id)
            % get device name for provided board id
            task_name = 'get_device_name';
            lib_name = BoardShim.load_lib();
            % no way to understand how it works in matlab used this link
            % https://nl.mathworks.com/matlabcentral/answers/131446-what-data-type-do-i-need-to-calllib-with-pointer-argument-char%
            [exit_code, device_name] = calllib(lib_name, task_name, board_id, blanks(4096), 4096);
            BoardShim.check_ec(exit_code, task_name);
        end
        
        function eeg_channels = get_eeg_channels(board_id)
            % get eeg channels for provided board id
            task_name = 'get_eeg_channels';
            num_channels = libpointer('int32Ptr', 0);
            lib_name = BoardShim.load_lib();
            data = libpointer('int32Ptr', zeros(1, 512));
            exit_code = calllib(lib_name, task_name, board_id, data, num_channels);
            BoardShim.check_ec(exit_code, task_name);
            eeg_channels = data.Value(1,1:num_channels.Value) + 1;
        end
        
        function exg_channels = get_exg_channels(board_id)
            % get exg channels for provided board id
            task_name = 'get_exg_channels';
            num_channels = libpointer('int32Ptr', 0);
            lib_name = BoardShim.load_lib();
            data = libpointer('int32Ptr', zeros(1, 512));
            exit_code = calllib(lib_name, task_name, board_id, data, num_channels);
            BoardShim.check_ec(exit_code, task_name);
            exg_channels = data.Value(1,1:num_channels.Value) + 1;
        end
        
        function emg_channels = get_emg_channels(board_id)
            % get emg channels for provided board id
            task_name = 'get_emg_channels';
            lib_name = BoardShim.load_lib();
            num_channels = libpointer('int32Ptr', 0);
            data = libpointer('int32Ptr', zeros(1, 512));
            exit_code = calllib(lib_name, task_name, board_id, data, num_channels);
            BoardShim.check_ec(exit_code, task_name);
            emg_channels = data.Value(1,1:num_channels.Value) + 1;
        end
        
        function ecg_channels = get_ecg_channels(board_id)
            % get ecg channels for provided board id
            task_name = 'get_ecg_channels';
            lib_name = BoardShim.load_lib();
            num_channels = libpointer('int32Ptr', 0);
            data = libpointer('int32Ptr', zeros(1, 512));
            exit_code = calllib(lib_name, task_name, board_id, data, num_channels);
            BoardShim.check_ec(exit_code, task_name);
            ecg_channels = data.Value(1,1:num_channels.Value) + 1;
        end
        
        function eog_channels = get_eog_channels(board_id)
            % get eog channels for provided board id
            task_name = 'get_eog_channels';
            lib_name = BoardShim.load_lib();
            num_channels = libpointer('int32Ptr', 0);
            data = libpointer('int32Ptr', zeros(1, 512));
            exit_code = calllib(lib_name, task_name, board_id, data, num_channels);
            BoardShim.check_ec(exit_code, task_name);
            eog_channels = data.Value(1,1:num_channels.Value) + 1;
        end
        
        function ppg_channels = get_ppg_channels(board_id)
            % get ppg channels for provided board id
            task_name = 'get_ppg_channels';
            num_channels = libpointer('int32Ptr', 0);
            data = libpointer('int32Ptr', zeros(1, 512));
            lib_name = BoardShim.load_lib();
            exit_code = calllib(lib_name, task_name, board_id, data, num_channels);
            BoardShim.check_ec(exit_code, task_name);
            ppg_channels = data.Value(1,1:num_channels.Value) + 1;
        end
        
        function eda_channels = get_eda_channels(board_id)
            % get eda channels for provided board id
            task_name = 'get_eda_channels';
            lib_name = BoardShim.load_lib();
            num_channels = libpointer('int32Ptr', 0);
            data = libpointer('int32Ptr', zeros(1, 512));
            exit_code = calllib(lib_name, task_name, board_id, data, num_channels);
            BoardShim.check_ec(exit_code, task_name);
            eda_channels = data.Value(1,1:num_channels.Value) + 1;
        end
        
        function accel_channels = get_accel_channels(board_id)
            % get accel channels for provided board id
            task_name = 'get_accel_channels';
            lib_name = BoardShim.load_lib();
            num_channels = libpointer('int32Ptr', 0);
            data = libpointer('int32Ptr', zeros(1, 512));
            exit_code = calllib(lib_name, task_name, board_id, data, num_channels);
            BoardShim.check_ec(exit_code, task_name);
            accel_channels = data.Value(1,1:num_channels.Value) + 1;
        end
        
        function analog_channels = get_analog_channels(board_id)
            % get analog channels for provided board id
            task_name = 'get_analog_channels';
            lib_name = BoardShim.load_lib();
            num_channels = libpointer('int32Ptr', 0);
            data = libpointer('int32Ptr', zeros(1, 512));
            exit_code = calllib(lib_name, task_name, board_id, data, num_channels);
            BoardShim.check_ec(exit_code, task_name);
            analog_channels = data.Value(1,1:num_channels.Value) + 1;
        end
        
        function other_channels = get_other_channels(board_id)
            % get other channels for provided board id
            task_name = 'get_other_channels';
            lib_name = BoardShim.load_lib();
            num_channels = libpointer('int32Ptr', 0);
            data = libpointer('int32Ptr', zeros(1, 512));
            exit_code = calllib(lib_name, task_name, board_id, data, num_channels);
            BoardShim.check_ec(exit_code, task_name);
            other_channels = data.Value(1,1:num_channels.Value) + 1;
        end
        
        function temperature_channels = get_temperature_channels(board_id)
            % get temperature channels for provided board id
            task_name = 'get_temperature_channels';
            lib_name = BoardShim.load_lib();
            num_channels = libpointer('int32Ptr', 0);
            data = libpointer('int32Ptr', zeros(1, 512));
            exit_code = calllib(lib_name, task_name, board_id, data, num_channels);
            BoardShim.check_ec(exit_code, task_name);
            temperature_channels = data.Value(1,1:num_channels.Value) + 1;
        end
        
        function resistance_channels = get_resistance_channels(board_id)
            % get resistance channels for provided board id
            task_name = 'get_resistance_channels';
            lib_name = BoardShim.load_lib();
            num_channels = libpointer('int32Ptr', 0);
            data = libpointer('int32Ptr', zeros(1, 512));
            exit_code = calllib(lib_name, task_name, board_id, data, num_channels);
            BoardShim.check_ec(exit_code, task_name);
            resistance_channels = data.Value(1,1:num_channels.Value) + 1;
        end
        
    end

    methods

        function obj = BoardShim(board_id, input_params)
            % BoardShim constructor
            obj.input_params_json = input_params.to_json();
            obj.board_id = int32(board_id);
            obj.master_board_id = obj.board_id;
            if((board_id == int32(BoardIDs.STREAMING_BOARD)) ||(board_id == int32(BoardIDs.PLAYBACK_FILE_BOARD)))
                double_val = str2double(input_params.other_info);
                if(isnan(double_val))
                    error("Write master board ID to other_info field");
                end
                obj.master_board_id = int32(double_val);
            end
        end

        function prepare_session(obj)
            % prepare BoardShim session
            task_name = 'prepare_session';
            lib_name = BoardShim.load_lib();
            exit_code = calllib(lib_name, task_name, obj.board_id, obj.input_params_json);
            BoardShim.check_ec(exit_code, task_name);
        end

        function response = config_board(obj, config)
            % send string to the board
            task_name = 'config_board';
            lib_name = BoardShim.load_lib();
            % no way to understand how it works in matlab used this link
            % https://nl.mathworks.com/matlabcentral/answers/131446-what-data-type-do-i-need-to-calllib-with-pointer-argument-char%
            [exit_code, tmp, response] = calllib(lib_name, task_name, config, blanks(4096), 4096, obj.board_id, obj.input_params_json);
            BoardShim.check_ec(exit_code, task_name);
        end

        function start_stream(obj, buffer_size, streamer_params)
            % start data acqusition
            task_name = 'start_stream';
            lib_name = BoardShim.load_lib();
            exit_code = calllib(lib_name, task_name, buffer_size, streamer_params, obj.board_id, obj.input_params_json);
            BoardShim.check_ec(exit_code, task_name);
        end

        function stop_stream(obj)
            % stop acqusition
            task_name = 'stop_stream';
            lib_name = BoardShim.load_lib();
            exit_code = calllib(lib_name, task_name, obj.board_id, obj.input_params_json);
            BoardShim.check_ec(exit_code, task_name);
        end

        function release_session(obj)
            % release session
            task_name = 'release_session';
            lib_name = BoardShim.load_lib();
            exit_code = calllib(lib_name, task_name, obj.board_id, obj.input_params_json);
            BoardShim.check_ec(exit_code, task_name);
        end
        
        function insert_marker(obj, value)
            % insert marker
            task_name = 'insert_marker';
            lib_name = BoardShim.load_lib();
            exit_code = calllib(lib_name, task_name, double(value), obj.board_id, obj.input_params_json);
            BoardShim.check_ec(exit_code, task_name);
        end

        function num_data_point = get_board_data_count(obj)
            % get amount of datapoints in internal buffer
            task_name = 'get_board_data_count';
            data_count = libpointer('int32Ptr', 0);
            lib_name = BoardShim.load_lib();
            exit_code = calllib(lib_name, task_name, data_count, obj.board_id, obj.input_params_json);
            BoardShim.check_ec(exit_code, task_name);
            num_data_point = data_count.value;
        end

        function data_buf = get_board_data(obj, num_datapoints)
            % get required amount of data by specifying it and remove it from the buffer
            task_name = 'get_board_data';
            data_count = obj.get_board_data_count();
            if (num_datapoints < 0)
                BoardShim.check_ec(int32(ExitCodes.INVALID_ARGUMENTS_ERROR), 'invalid num_datapoints');
            else
                if (data_count >= num_datapoints)
                    data_count = num_datapoints;
                end
            end
            num_rows = BoardShim.get_num_rows(obj.master_board_id);
            lib_name = BoardShim.load_lib();
            data = libpointer('doublePtr', zeros(1, data_count * num_rows));
            exit_code = calllib(lib_name, task_name, data_count, data, obj.board_id, obj.input_params_json);
            BoardShim.check_ec(exit_code, task_name);
            data_buf = transpose(reshape(data.Value, [data_count, num_rows]));
        end

        function data_buf = get_current_board_data(obj, num_samples)
            % get latest datapoints, doesnt remove it from internal buffer
            task_name = 'get_current_board_data';
            data_count = libpointer('int32Ptr', 0);
            num_rows = BoardShim.get_num_rows(obj.master_board_id);
            lib_name = BoardShim.load_lib();
            data = libpointer('doublePtr', zeros(1, num_samples * num_rows));
            exit_code = calllib(lib_name, task_name, num_samples, data, data_count, obj.board_id, obj.input_params_json);
            BoardShim.check_ec(exit_code, task_name);
            data_buf = transpose(reshape(data.Value(1,1:data_count.Value * num_rows), [data_count.Value, num_rows]));
        end
        
        function prepared = is_prepared(obj)
           % check if brainflow session prepared
           task_name = 'is_prepared';
           res = libpointer('int32Ptr', 0);
           lib_name = BoardShim.load_lib();
           exit_code = calllib(lib_name, task_name, res, obj.board_id, obj.input_params_json);
           BoardShim.check_ec(exit_code, task_name);
           prepared = boolean(res.value);
        end
        
    end
    
end
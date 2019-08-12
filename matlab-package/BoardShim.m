classdef BoardShim
    properties
        libname
        num_channels
        board_id
        port_name
        exit_codes
    end
    methods
        function obj = BoardShim (board_id, port_name)
            obj.exit_codes = containers.Map ('KeyType', 'int32', 'ValueType', 'any');
            obj.exit_codes(0) = 'STATUS_OK';
            obj.exit_codes(1) = 'PORT_ALREADY_OPEN_ERROR';
            obj.exit_codes(2) = 'UNABLE_TO_OPEN_PORT_ERROR';
            obj.exit_codes(3) = 'SET_PORT_ERROR';
            obj.exit_codes(4) = 'BOARD_WRITE_ERROR';
            obj.exit_codes(5) = 'INCOMMING_MSG_ERROR';
            obj.exit_codes(6) = 'INITIAL_MSG_ERROR';
            obj.exit_codes(7) = 'BOARD_NOT_READY_ERROR';
            obj.exit_codes(8) = 'STREAM_ALREADY_RUN_ERROR';
            obj.exit_codes(9) = 'INVALID_BUFFER_SIZE_ERROR';
            obj.exit_codes(10) = 'STREAM_THREAD_ERROR';
            obj.exit_codes(11) = 'STREAM_THREAD_IS_NOT_RUNNING';
            obj.exit_codes(12) = 'EMPTY_BUFFER_ERROR';
            obj.exit_codes(13) = 'INVALID_ARGUMENTS_ERROR';
            obj.exit_codes(14) = 'UNSUPPORTED_BOARD_ERROR';
            obj.exit_codes(15) = 'BOARD_NOT_CREATED_ERROR';
            obj.exit_codes(16) = 'ANOTHER_BOARD_IS_CREATED_ERROR';
            obj.exit_codes(17) = 'GENERAL_ERROR';
            obj.exit_codes(18) = 'SYNC_TIMEOUT_ERROR';
            if ismac
                addpath('brainflow/lib/APPLE')
                obj.libname = 'libBoardController';
            elseif isunix
                addpath('brainflow/lib/UNIX')
                obj.libname = 'libBoardController';
            else
                addpath('brainflow/lib/MSVC')
                obj.libname = 'BoardController';
            end
            obj.port_name = port_name;
            obj.board_id = int32 (board_id);
            if board_id == int32 (BoardIDs.CYTON_BOARD)
                obj.num_channels = 12;
            elseif board_id == int32 (BoardIDs.GANGLION_BOARD)
                obj.num_channels = 8;
            elseif board_id == int32 (BoardIDs.SYNTHETIC_BOARD)
                obj.num_channels = 12;
            elseif board_id == int32 (BoardIDs.CYTON_DAISY_BOARD)
                obj.num_channels = 20;
            loadlibrary (obj.libname, 'board_controller.h') 
            end
            
        end

        function check_ec (obj, ec, task_name)
            status = obj.exit_codes(ec);
            if strcmp(status,'STATUS_OK')
                disp([task_name, ': ', status])
            else
                error ('Non zero ec: %d, desc: %s', ec, status)
            end
        end

        function prepare_session (obj)
            task_name = 'prepare_session';
            exit_code = calllib (obj.libname, task_name, obj.board_id, obj.port_name);
            obj.check_ec(exit_code, task_name);
        end

        function start_stream (obj, buffer_size)
            task_name = 'start_stream';
            exit_code = calllib (obj.libname, task_name, buffer_size);
            obj.check_ec(exit_code, task_name);
        end

        function stop_stream (obj)
            task_name = 'stop_stream';
            exit_code = calllib (obj.libname, task_name);
            obj.check_ec(exit_code, task_name);
        end

        function release_session (obj)
            task_name = 'release_session';
            exit_code = calllib (obj.libname, task_name);
            obj.check_ec(exit_code, task_name);
        end

        function num_data_point = get_board_data_count (obj)
            task_name = 'get_board_data_count';
            data_count = libpointer ('int32Ptr', 0);
            exit_code = calllib(obj.libname, task_name, data_count);
            obj.check_ec(exit_code, task_name);
            num_data_point = data_count.value;
        end

        function [data_buf, ts_buf] = get_board_data (obj)
            task_name = 'get_board_data';
            data_count = obj.get_board_data_count ();
            data = libpointer ('singlePtr', zeros (1, data_count * obj.num_channels));
            ts = libpointer ('doublePtr', zeros (1, data_count));
            exit_code = calllib(obj.libname, task_name, data_count, data, ts);
            obj.check_ec(exit_code, task_name);
            data_buf = transpose(reshape(data.Value, [obj.num_channels, data_count]));
            ts_buf = ts.Value;
        end

        function [data_buf, ts_buf] = get_current_board_data (obj, num_samples)
            task_name = 'get_current_board_data';
            data_count = libpointer ('int32Ptr', 0);
            data = libpointer ('singlePtr', zeros (1, num_samples * obj.num_channels));
            ts = libpointer ('doublePtr', zeros (1, num_samples));
            exit_code = calllib(obj.libname, task_name, num_samples, data, ts, data_count);
            obj.check_ec(exit_code, task_name);
            data_buf = transpose(reshape(data.Value(1,1:data_count.Value * obj.num_channels), [obj.num_channels, data_count.Value]));
            ts_buf = ts.Value(1,1:data_count.Value);
        end
    end
end
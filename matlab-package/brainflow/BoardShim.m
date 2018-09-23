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
            obj.exit_codes = containers.Map ('KeyType', 'int32', 'ValueType', 'any')
            obj.exit_codes(0) = 'STATUS_OK'
            obj.exit_codes(1) = 'PORT_ALREADY_OPEN_ERROR'
            obj.exit_codes(2) = 'UNABLE_TO_OPEN_PORT_ERROR'
            obj.exit_codes(3) = 'SET_PORT_ERROR'
            obj.exit_codes(4) = 'BOARD_WRITE_ERROR'
            obj.exit_codes(5) = 'INCOMMING_MSG_ERROR'
            obj.exit_codes(6) = 'INITIAL_MSG_ERROR'
            obj.exit_codes(7) = 'BOARD_NOT_READY_ERROR'
            obj.exit_codes(8) = 'STREAM_ALREADY_RUN_ERROR'
            obj.exit_codes(9) = 'INVALID_BUFFER_SIZE_ERROR'
            obj.exit_codes(10) = 'STREAM_THREAD_ERROR'
            obj.exit_codes(11) = 'STREAM_THREAD_IS_NOT_RUNNING'
            obj.exit_codes(12) = 'EMPTY_BUFFER_ERROR'
            obj.exit_codes(13) = 'INVALID_ARGUMENTS_ERROR'
            obj.exit_codes(14) = 'UNSUPPORTED_BOARD_ERROR'
            obj.exit_codes(15) = 'BOARD_NOT_CREATED_ERROR'
            if isunix
                obj.libname = 'libBoardController'
            else
                obj.libname = 'BoardController'
            end
            obj.port_name = port_name
            obj.board_id = uint32 (board_id)
            if board_id == uint32(BoardsIds.CYTHON_BOARD)
                obj.num_channels = 12
            end
            loadlibrary (obj.libname, 'board_controller.h')
        end
   
        function check_ec (obj, ec)
            if ec ~= 0
                error ('Non zero ec: %d, desc: %s', ec, obj.exit_codes(ec))
            end
        end

        function exit_code = prepare_session (obj)
            exit_code = calllib (obj.libname,'prepare_session', obj.board_id, obj.port_name);
        end

        function exit_code = start_stream (obj, buffer_size)
            exit_code = calllib (obj.libname,'start_stream', buffer_size);
        end

        function exit_code = stop_stream (obj)
            exit_code = calllib (obj.libname, 'stop_stream');
        end

        function exit_code = release_session (obj)
            exit_code = calllib (obj.libname, 'release_session');
        end

        function [exit_code, num_data_point] = get_board_data_count (obj)
            data_count = libpointer ('int32Ptr', 0)
            exit_code = calllib(obj.libname, 'get_board_data_count', data_count)
            num_data_point = data_count.value
        end

        function [exit_code, data_buf, ts_buf] = get_board_data (obj)
            [exit_code, data_count] = obj.get_board_data_count ()
            if exit_code ~= 0
                return
            end
            data = libpointer ('singlePtr', zeros (1, data_count * obj.num_channels))
            ts = libpointer ('doublePtr', zeros (1, data_count))
            exit_code = calllib(obj.libname, 'get_board_data', data_count, data, ts)
            data_buf = transpose(reshape(data.Value, [obj.num_channels, data_count]))
            ts_buf = ts.Value
        end
        
        function [exit_code, data_buf, ts_buf] = get_current_board_data (obj, num_samples)
            data_count = libpointer ('int32Ptr', 0)
            data = libpointer ('singlePtr', zeros (1, num_samples * obj.num_channels))
            ts = libpointer ('doublePtr', zeros (1, num_samples))
            exit_code = calllib(obj.libname, 'get_current_board_data', num_samples, data, ts, data_count)
            data_buf = transpose(reshape(data.Value(1,1:data_count.Value * obj.num_channels), [obj.num_channels, data_count.Value]))
            ts_buf = ts.Value(1,1:data_count.Value)
        end
    end
end
classdef MLModel
    % MLModel for inference
    properties
        input_json
        input_params
    end

    properties(Access = private)
        model_json
        output_capacity
    end

    methods(Static)
        
        function lib_name = load_lib()
            if ispc
                if not(libisloaded('MLModule'))
                    loadlibrary('MLModule.dll', 'ml_module.h');
                end
                lib_name = 'MLModule';
            elseif ismac
                if not(libisloaded('libMLModule'))
                    loadlibrary('libMLModule.dylib', 'ml_module.h');
                end
                lib_name = 'libMLModule';
            elseif isunix
                if not(libisloaded('libMLModule'))
                    loadlibrary('libMLModule.so', 'ml_module.h');
                end
                lib_name = 'libMLModule';
            else
                error('OS not supported!')
            end
        end
        
        function check_ec(ec, task_name)
            if(ec ~= int32(BrainFlowExitCodes.STATUS_OK))
                error('Non zero ec: %d, for task: %s', ec, task_name)
            end
        end
        
        function release_all()
            % release all sessions
            task_name = 'release_all';
            lib_name = MLModel.load_lib();
            exit_code = calllib(lib_name, task_name);
            MLModel.check_ec(exit_code, task_name);
        end

        function log_message(log_level, message)
            % write message to ML logger
            task_name = 'log_message_ml_module';
            lib_name = MLModel.load_lib();
            exit_code = calllib(lib_name, task_name, log_level, message);
            MLModel.check_ec(exit_code, task_name);
        end
        
        function set_log_level(log_level)
            % set log level for MLModel
            task_name = 'set_log_level_ml_module';
            lib_name = MLModel.load_lib();
            exit_code = calllib(lib_name, task_name, log_level);
            MLModel.check_ec(exit_code, task_name);
        end

        function set_log_file(log_file)
            % set log file for MLModel
            task_name = 'set_log_file_ml_module';
            lib_name = MLModel.load_lib();
            exit_code = calllib(lib_name, task_name, log_file);
            MLModel.check_ec(exit_code, task_name);
        end

        function enable_ml_logger()
            % enable logger with level INFO
            MLModel.set_log_level(int32(2))
        end

        function enable_dev_ml_logger()
            % enable logger with level TRACE
            MLModel.set_log_level(int32(0))
        end

        function disable_ml_logger()
            % disable logger
            MLModel.set_log_level(int32(6))
        end
        
        function version = get_version()
            % get version
            task_name = 'get_version_ml_module';
            lib_name = MLModel.load_lib();
            % no way to understand how it works in matlab, used this link
            % https://nl.mathworks.com/matlabcentral/answers/131446-what-data-type-do-i-need-to-calllib-with-pointer-argument-char%
            [exit_code, version] = calllib(lib_name, task_name, blanks(64), 64, 64);
            MLModel.check_ec(exit_code, task_name);
        end
   
    end

    methods

        function obj = MLModel(params)
            capacity = params.max_array_size;
            if ~isnumeric(capacity) || ~isreal(capacity) || ~isscalar(capacity) || ...
                    ~isfinite(capacity) || capacity < 1 || capacity ~= fix(capacity) || ...
                    capacity > double(intmax('int32'))
                error('max_array_size must be a positive int32 integer');
            end
            obj.input_json = params.to_json();
            obj.input_params = params;
            % Public parameter copies remain available for inspection. Native
            % lookup and allocation always use the original matching snapshot.
            obj.model_json = obj.input_json;
            obj.output_capacity = double(capacity);
        end

        function prepare(obj)
            % prepare model
            task_name = 'prepare';
            lib_name = MLModel.load_lib();
            exit_code = calllib(lib_name, task_name, obj.model_json);
            MLModel.check_ec(exit_code, task_name);
        end
        
        function release(obj)
            % release model
            task_name = 'release';
            lib_name = MLModel.load_lib();
            exit_code = calllib(lib_name, task_name, obj.model_json);
            MLModel.check_ec(exit_code, task_name);
        end

        function score = predict(obj, input_data)
            % perform inference for input data
            if ~isnumeric(input_data) || ~isreal(input_data) || ~isvector(input_data) || ...
                    isempty(input_data) || numel(input_data) > double(intmax('int32')) || ...
                    any(~isfinite(input_data(:)))
                error('Prediction input must be a nonempty finite real vector');
            end
            input_data = full(double(input_data(:).'));
            task_name = 'predict';
            lib_name = MLModel.load_lib();
            score_temp = libpointer('doublePtr', zeros(1, obj.output_capacity, 'double'));
            len = libpointer('int32Ptr', 0);
            input_data_temp = libpointer('doublePtr', input_data);
            exit_code = calllib(lib_name, task_name, input_data_temp, numel(input_data), score_temp, len, obj.model_json);
            MLModel.check_ec(exit_code, task_name);
            output_length = double(len.Value);
            if output_length < 0 || output_length > obj.output_capacity
                error('Native prediction length exceeds the output buffer');
            end
            score = score_temp.Value(1, 1:output_length);
        end
        
    end
    
end

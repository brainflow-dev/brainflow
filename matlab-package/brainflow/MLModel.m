classdef MLModel
    % MLModel for inference
    properties
        input_json
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
            if(ec ~= int32(ExitCodes.STATUS_OK))
                error('Non zero ec: %d, for task: %s', ec, task_name)
            end
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
   
    end

    methods

        function obj = MLModel(params)
            obj.input_json = params.to_json();
        end

        function prepare(obj)
            % prepare model
            task_name = 'prepare';
            lib_name = MLModel.load_lib();
            exit_code = calllib(lib_name, task_name, obj.input_json);
            MLModel.check_ec(exit_code, task_name);
        end
        
        function release(obj)
            % release model
            task_name = 'release';
            lib_name = MLModel.load_lib();
            exit_code = calllib(lib_name, task_name, obj.input_json);
            MLModel.check_ec(exit_code, task_name);
        end

        function score = predict(obj, input_data)
            % perform inference for input data
            task_name = 'predict';
            lib_name = MLModel.load_lib();
            score_temp = libpointer('doublePtr', 0.0);
            input_data_temp = libpointer('doublePtr', input_data);
            exit_code = calllib(lib_name, task_name, input_data_temp, size(input_data, 2), score_temp, obj.input_json);
            MLModel.check_ec(exit_code, task_name);
            score = score_temp.Value;
        end
        
    end
    
end
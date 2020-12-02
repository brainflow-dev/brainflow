
@enum BrainFlowLogLib begin

    BOARD_CONTROLLER = 0
    DATA_HANDLER = 1
    ML_MODULE = 2

end


@enum LogLevels begin

    LEVEL_TRACE = 0
    LEVEL_DEBUG = 1
    LEVEL_INFO = 2
    LEVEL_WARN = 3
    LEVEL_ERROR = 4
    LEVEL_CRITICAL = 5
    LEVEL_OFF = 6

end

enable_brainflow_logger(log_lib::BrainFlowLogLib) = enable_brainflow_logger(Integer(log_lib))
function enable_brainflow_logger(log_lib::Integer)
    set_log_level(Integer(LEVEL_INFO), log_lib)
end

enable_dev_brainflow_logger(log_lib::BrainFlowLogLib) = enable_dev_brainflow_logger(Integer(log_lib))
function enable_dev_brainflow_logger(log_lib::Integer)
    set_log_level(Integer(LEVEL_TRACE), log_lib)
end

disable_brainflow_logger(log_lib::BrainFlowLogLib) = disable_brainflow_logger(Integer(log_lib))
function disable_brainflow_logger(log_lib::Integer)
    set_log_level(Integer(LEVEL_OFF), log_lib)
end


# available only for BoardController, no need to provide log_lib
function log_message(log_level::Integer, message::String)
    ec = Integer(STATUS_OK)
    ec = ccall((:log_message, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{UInt8}), Int32(log_level), message)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in log_message ", ec), ec))
    end
end


function set_log_file(log_file::String, log_lib::Integer)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Integer(log_lib) == Integer(BOARD_CONTROLLER)
        ec = ccall((:set_log_file, BOARD_CONTROLLER_INTERFACE), Cint, (Ptr{UInt8},), log_file)
    elseif Integer(log_lib) == Integer(DATA_HANDLER)
        ec = ccall((:set_log_file, DATA_HANDLER_INTERFACE), Cint, (Ptr{UInt8},), log_file)
    elseif Integer(log_lib) == Integer(ML_MODULE)
        ec = ccall((:set_log_file, ML_MODULE_INTERFACE), Cint, (Ptr{UInt8},), log_file)
    else
        ec = Integer(INVALID_ARGUMENTS_ERROR)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in set_log_file ", ec), ec))
    end
end



function set_log_level(log_level::Integer, log_lib::Integer)
    ec = Integer(STATUS_OK)
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Integer(log_lib) == Integer(BOARD_CONTROLLER)
        ec = ccall((:set_log_level, BOARD_CONTROLLER_INTERFACE), Cint, (Cint,), Int32(log_level))
    elseif Integer(log_lib) == Integer(DATA_HANDLER)
        ec = ccall((:set_log_level, DATA_HANDLER_INTERFACE), Cint, (Cint,), Int32(log_level))
    elseif Integer(log_lib) == Integer(ML_MODULE)
        ec = ccall((:set_log_level, ML_MODULE_INTERFACE), Cint, (Cint,), Int32(log_level))
    else
        ec = Integer(INVALID_ARGUMENTS_ERROR)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in set_log_level ", ec), ec))
    end
end
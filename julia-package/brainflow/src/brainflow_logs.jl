AnyIntType = Union{Int8, Int32, Int64, Int128, Int}


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

function enable_board_logger(log_lib::AnyIntType)
    set_log_level(Integer(LEVEL_INFO), Integer(log_lib))
end


function enable_dev_board_logger(log_lib::AnyIntType)
    set_log_level(Integer(LEVEL_TRACE), Integer(log_lib))
end


function disable_board_logger(log_lib::AnyIntType)
    set_log_level(Integer(LEVEL_OFF), Integer(log_lib))
end


# available only for BoardController, no need to provide log_lib
function log_message(log_level::AnyIntType, message::String)
    ec = Integer(STATUS_OK)
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:log_message, "BoardController.dll"), Cint, (Cint, Ptr{UInt8}), Int32(log_level), message)
    elseif Sys.isapple()
        ec = ccall((:log_message, "libBoardController.dylib"), Cint, (Cint, Ptr{UInt8}), Int32(log_level), message)
    else
        ec = ccall((:log_message, "libBoardController.so"), Cint, (Cint, Ptr{UInt8}), Int32(log_level), message)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in log_message ", ec), ec))
    end
end


function set_log_file(log_file::String, log_lib::AnyIntType)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Integer(log_lib) == Integer(BOARD_CONTROLLER)
        if Sys.iswindows()
            ec = ccall((:set_log_file, "BoardController.dll"), Cint, (Ptr{UInt8},), log_file)
        elseif Sys.isapple()
            ec = ccall((:set_log_file, "libBoardController.dylib"), Cint, (Ptr{UInt8},), log_file)
        else
            ec = ccall((:set_log_file, "libBoardController.so"), Cint, (Ptr{UInt8},), log_file)
        end
    elseif Integer(log_lib) == Integer(DATA_HANDLER)
        if Sys.iswindows()
            ec = ccall((:set_log_file, "DataHandler.dll"), Cint, (Ptr{UInt8},), log_file)
        elseif Sys.isapple()
            ec = ccall((:set_log_file, "libDataHandler.dylib"), Cint, (Ptr{UInt8},), log_file)
        else
            ec = ccall((:set_log_file, "libDataHandler.so"), Cint, (Ptr{UInt8},), log_file)
        end
    elseif Integer(log_lib) == Integer(ML_MODULE)
        if Sys.iswindows()
            ec = ccall((:set_log_file, "MLModule.dll"), Cint, (Ptr{UInt8},), log_file)
        elseif Sys.isapple()
            ec = ccall((:set_log_file, "libMLModule.dylib"), Cint, (Ptr{UInt8},), log_file)
        else
            ec = ccall((:set_log_file, "libMLModule.so"), Cint, (Ptr{UInt8},), log_file)
        end
    else
        ec = Integer(INVALID_ARGUMENTS_ERROR)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in set_log_file ", ec), ec))
    end
end



function set_log_level(log_level::AnyIntType, log_lib::AnyIntType)
    ec = Integer(STATUS_OK)
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Integer(log_lib) == Integer(BOARD_CONTROLLER)
        if Sys.iswindows()
            ec = ccall((:set_log_level, "BoardController.dll"), Cint, (Cint,), Int32(log_level))
        elseif Sys.isapple()
            ec = ccall((:set_log_level, "libBoardController.dylib"), Cint, (Cint,), Int32(log_level))
        else
            ec = ccall((:set_log_level, "libBoardController.so"), Cint, (Cint,), Int32(log_level))
        end
    elseif Integer(log_lib) == Integer(DATA_HANDLER)
        if Sys.iswindows()
            ec = ccall((:set_log_level, "DataHandler.dll"), Cint, (Cint,), Int32(log_level))
        elseif Sys.isapple()
            ec = ccall((:set_log_level, "libDataHandler.dylib"), Cint, (Cint,), Int32(log_level))
        else
            ec = ccall((:set_log_level, "libDataHandler.so"), Cint, (Cint,), Int32(log_level))
        end
    elseif Integer(log_lib) == Integer(ML_MODULE)
        if Sys.iswindows()
            ec = ccall((:set_log_level, "MLModule.dll"), Cint, (Cint,), Int32(log_level))
        elseif Sys.isapple()
            ec = ccall((:set_log_level, "libMLModule.dylib"), Cint, (Cint,), Int32(log_level))
        else
            ec = ccall((:set_log_level, "libMLModule.so"), Cint, (Cint,), Int32(log_level))
        end
    else
        ec = Integer(INVALID_ARGUMENTS_ERROR)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in set_log_level ", ec), ec))
    end
end
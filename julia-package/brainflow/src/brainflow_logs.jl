
abstract type BrainFlowLib end
struct BoardControllerLib <: BrainFlowLib end
struct DataHandlerLib <: BrainFlowLib end
struct MlModuleLib <: BrainFlowLib end
library_path(::BoardControllerLib) = BOARD_CONTROLLER_INTERFACE
library_path(::DataHandlerLib) = DATA_HANDLER_INTERFACE
library_path(::MlModuleLib) = ML_MODULE_INTERFACE

# during refactoring
const BOARD_CONTROLLER = BoardControllerLib()
const DATA_HANDLER = DataHandlerLib()
const ML_MODULE = MlModuleLib()

@enum LogLevels begin

    LEVEL_TRACE = 0
    LEVEL_DEBUG = 1
    LEVEL_INFO = 2
    LEVEL_WARN = 3
    LEVEL_ERROR = 4
    LEVEL_CRITICAL = 5
    LEVEL_OFF = 6

end

enable_brainflow_logger(log_lib::BrainFlowLib) = set_log_level(Integer(LEVEL_INFO), log_lib)
enable_dev_brainflow_logger(log_lib::BrainFlowLib) = set_log_level(Integer(LEVEL_TRACE), log_lib)
disable_brainflow_logger(log_lib::BrainFlowLib) = set_log_level(Integer(LEVEL_OFF), log_lib)

# available only for BoardController, no need to provide log_lib
@brainflow_rethrow function log_message(log_level::Integer, message::String)
    ccall((:log_message, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{UInt8}), Int32(log_level), message)
end

# TODO: convert into a macro?
@brainflow_rethrow function set_log_file(log_file, log_lib::BrainFlowLib)
    lib_cglobal = log_file_cglobal(log_lib)
    ccall(lib_cglobal, Cint, (Ptr{UInt8},), log_file)
end

# need to hardcode the cglobal input symbols, related to https://github.com/JuliaLang/julia/issues/29602
log_file_cglobal(::BoardControllerLib) = cglobal((:set_log_file, BOARD_CONTROLLER_INTERFACE)) 
log_file_cglobal(::DataHandlerLib) = cglobal((:set_log_file, DATA_HANDLER_INTERFACE)) 
log_file_cglobal(::MlModuleLib) = cglobal((:set_log_file, ML_MODULE_INTERFACE)) 

@brainflow_rethrow function set_log_level(log_level::Integer, log_lib::BrainFlowLib)
    lib_cglobal = log_level_cglobal(log_lib)
    ccall(lib_cglobal, Cint, (Cint,), Int32(log_level))
end

# need to hardcode the cglobal input symbols, related to https://github.com/JuliaLang/julia/issues/29602
log_level_cglobal(::BoardControllerLib) = cglobal((:set_log_level, BOARD_CONTROLLER_INTERFACE)) 
log_level_cglobal(::DataHandlerLib) = cglobal((:set_log_level, DATA_HANDLER_INTERFACE)) 
log_level_cglobal(::MlModuleLib) = cglobal((:set_log_level, ML_MODULE_INTERFACE)) 


using MacroTools

struct BrainFlowError <: Exception
    msg::String
    ec::Int32
end

@enum BrainflowExitCodes begin

    STATUS_OK = 0
    PORT_ALREADY_OPEN_ERROR = 1
    UNABLE_TO_OPEN_PORT_ERROR = 2
    SER_PORT_ERROR = 3
    BOARD_WRITE_ERROR = 4
    INCOMMING_MSG_ERROR = 5
    INITIAL_MSG_ERROR = 6
    BOARD_NOT_READY_ERROR = 7
    STREAM_ALREADY_RUN_ERROR = 8
    INVALID_BUFFER_SIZE_ERROR = 9
    STREAM_THREAD_ERROR = 10
    STREAM_THREAD_IS_NOT_RUNNING = 11
    EMPTY_BUFFER_ERROR = 12
    INVALID_ARGUMENTS_ERROR = 13
    UNSUPPORTED_BOARD_ERROR = 14
    BOARD_NOT_CREATED_ERROR = 15
    ANOTHER_BOARD_IS_CREATED_ERROR = 16
    GENERAL_ERROR = 17
    SYNC_TIMEOUT_ERROR = 18
    JSON_NOT_FOUND_ERROR = 19
    NO_SUCH_DATA_IN_JSON_ERROR = 20
    CLASSIFIER_IS_NOT_PREPARED_ERROR = 21
    ANOTHER_CLASSIFIER_IS_PREPARED_ERROR = 22
    UNSUPPORTED_CLASSIFIER_AND_METRIC_COMBINATION_ERROR = 23
end

"""
    @brainflow_rethrow

Macro that generates the brainflow error handling automatically after a ccall.

# Example
```
@brainflow_rethrow function prepare_session(board_shim::BoardShim)
    ccall((:prepare_session, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{UInt8}), board_shim.board_id, board_shim.input_json)
end
```

"""
macro brainflow_rethrow(defun)
    def = MacroTools.splitdef(defun)
    body = def[:body]
    name = string(def[:name])

    # find the ccall
    # assuming there is only 1 ccall
    # assuming you do NOT make an assignment, like x_ = ccall(xs__)
    args = body.args
    n_args = length(args)
    ccall_index = findfirst(x -> MacroTools.@capture(x, ccall(xs__)), args)

    # use the function name in the error message
    wrapup_ccall = quote 
        if ec != Integer(STATUS_OK)
            msg = string("Error in ", $(name), " ", BrainFlow.BrainflowExitCodes(Integer(ec)))
            throw(BrainFlowError(msg, Integer(ec)))
        end
    end

    # wrapping around the original ccall
    new_body = quote 
        $(args[1:ccall_index-1]...)
        ec = Integer(STATUS_OK)
        ec = $(args[ccall_index])
        $wrapup_ccall
        $(args[ccall_index+1:end]...)
    end

    def[:body] = new_body
    esc(MacroTools.combinedef(def))
end
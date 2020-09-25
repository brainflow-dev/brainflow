import JSON


AnyIntType = Union{Int8, Int32, Int64, Int128, Int}


@enum BoardIds begin

    STREAMING_BOARD = -2
    SYNTHETIC_BOARD = -1
    CYTON_BOARD = 0
    GANGLION_BOARD = 1
    CYTON_DAISY_BOARD = 2
    NOVAXR_BOARD = 3
    GANGLION_WIFI_BOARD = 4
    CYTON_WIFI_BOARD = 5
    CYTON_DAISY_WIFI_BOARD = 6
    BRAINBIT_BOARD = 7
    UNICORN_BOARD = 8
    CALLIBRI_EEG_BOARD = 9
    CALLIBRI_EMG_BOARD = 10
    CALLIBRI_ECG_BOARD = 11
    FASCIA_BOARD = 12
    NOTION_1_BOARD = 13
    NOTION_2_BOARD = 14
    IRONBCI_BOARD = 15

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


@enum IpProtocolType begin

    NONE = 0
    UDP = 1
    TCP = 2

end


struct BrainFlowError <: Exception
    msg::String
    ec::Int32
end


mutable struct BrainFlowInputParams

    serial_port::String
    mac_address::String
    ip_address::String
    ip_port::Int32
    ip_protocol::Int32
    other_info::String
    timeout::Int32
    serial_number::String

    function BrainFlowInputParams()
        new("", "", "", 0, Integer(NONE), "", 0, "")
    end

end


function get_timestamp_channel(board_id::AnyIntType)
    channel = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:get_timestamp_channel, "BoardController.dll"), Cint, (Cint, Ptr{Cint}), Int32(board_id), channel)
    elseif Sys.isapple()
        ec = ccall((:get_timestamp_channel, "libBoardController.dylib"), Cint, (Cint, Ptr{Cint}), Int32(board_id), channel)
    else
        ec = ccall((:get_timestamp_channel, "libBoardController.so"), Cint, (Cint, Ptr{Cint}), Int32(board_id), channel)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channel[1] + 1
    value
end


function get_package_num_channel(board_id::AnyIntType)
    channel = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:get_package_num_channel, "BoardController.dll"), Cint, (Cint, Ptr{Cint}), Int32(board_id), channel)
    elseif Sys.isapple()
        ec = ccall((:get_package_num_channel, "libBoardController.dylib"), Cint, (Cint, Ptr{Cint}), Int32(board_id), channel)
    else
        ec = ccall((:get_package_num_channel, "libBoardController.so"), Cint, (Cint, Ptr{Cint}), Int32(board_id), channel)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channel[1] + 1
    value
end


function get_battery_channel(board_id::AnyIntType)
    channel = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:get_battery_channel, "BoardController.dll"), Cint, (Cint, Ptr{Cint}), Int32(board_id), channel)
    elseif Sys.isapple()
        ec = ccall((:get_battery_channel, "libBoardController.dylib"), Cint, (Cint, Ptr{Cint}), Int32(board_id), channel)
    else
        ec = ccall((:get_battery_channel, "libBoardController.so"), Cint, (Cint, Ptr{Cint}), Int32(board_id), channel)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channel[1] + 1
    value
end


function get_sampling_rate(board_id::AnyIntType)
    val = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:get_sampling_rate, "BoardController.dll"), Cint, (Cint, Ptr{Cint}), Int32(board_id), val)
    elseif Sys.isapple()
        ec = ccall((:get_sampling_rate, "libBoardController.dylib"), Cint, (Cint, Ptr{Cint}), Int32(board_id), val)
    else
        ec = ccall((:get_sampling_rate, "libBoardController.so"), Cint, (Cint, Ptr{Cint}), Int32(board_id), val)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    value = val[1]
    value
end


function get_num_rows(board_id::AnyIntType)
    val = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:get_num_rows, "BoardController.dll"), Cint, (Cint, Ptr{Cint}), Int32(board_id), val)
    elseif Sys.isapple()
        ec = ccall((:get_num_rows, "libBoardController.dylib"), Cint, (Cint, Ptr{Cint}), Int32(board_id), val)
    else
        ec = ccall((:get_num_rows, "libBoardController.so"), Cint, (Cint, Ptr{Cint}), Int32(board_id), val)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # here dont need to add 1, last element included
    value = val[1]
    value
end


function get_eeg_names(board_id::AnyIntType)
    names_string = Vector{Cuchar}(undef, 4096)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:get_eeg_names, "BoardController.dll"), Cint, (Cint, Ptr{UInt8}, Ptr{Cint}), Int32(board_id), names_string, len)
    elseif Sys.isapple()
        ec = ccall((:get_eeg_names, "libBoardController.dylib"), Cint, (Cint, Ptr{UInt8}, Ptr{Cint}), Int32(board_id), names_string, len)
    else
        ec = ccall((:get_eeg_names, "libBoardController.so"), Cint, (Cint, Ptr{UInt8}, Ptr{Cint}), Int32(board_id), names_string, len)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    sub_string = String(names_string)[1:len[1]]
    value = split(sub_string, ',')
    value
end


function get_eeg_channels(board_id::AnyIntType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:get_eeg_channels, "BoardController.dll"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    elseif Sys.isapple()
        ec = ccall((:get_eeg_channels, "libBoardController.dylib"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    else
        ec = ccall((:get_eeg_channels, "libBoardController.so"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


function get_exg_channels(board_id::AnyIntType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:get_exg_channels, "BoardController.dll"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    elseif Sys.isapple()
        ec = ccall((:get_exg_channels, "libBoardController.dylib"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    else
        ec = ccall((:get_exg_channels, "libBoardController.so"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


function get_emg_channels(board_id::AnyIntType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:get_emg_channels, "BoardController.dll"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    elseif Sys.isapple()
        ec = ccall((:get_emg_channels, "libBoardController.dylib"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    else
        ec = ccall((:get_emg_channels, "libBoardController.so"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


function get_ecg_channels(board_id::AnyIntType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:get_ecg_channels, "BoardController.dll"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    elseif Sys.isapple()
        ec = ccall((:get_ecg_channels, "libBoardController.dylib"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    else
        ec = ccall((:get_ecg_channels, "libBoardController.so"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


function get_eog_channels(board_id::AnyIntType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:get_eog_channels, "BoardController.dll"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    elseif Sys.isapple()
        ec = ccall((:get_eog_channels, "libBoardController.dylib"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    else
        ec = ccall((:get_eog_channels, "libBoardController.so"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


function get_eda_channels(board_id::AnyIntType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:get_eda_channels, "BoardController.dll"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    elseif Sys.isapple()
        ec = ccall((:get_eda_channels, "libBoardController.dylib"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    else
        ec = ccall((:get_eda_channels, "libBoardController.so"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


function get_ppg_channels(board_id::AnyIntType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:get_ppg_channels, "BoardController.dll"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    elseif Sys.isapple()
        ec = ccall((:get_ppg_channels, "libBoardController.dylib"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    else
        ec = ccall((:get_ppg_channels, "libBoardController.so"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


function get_accel_channels(board_id::AnyIntType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:get_accel_channels, "BoardController.dll"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    elseif Sys.isapple()
        ec = ccall((:get_accel_channels, "libBoardController.dylib"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    else
        ec = ccall((:get_accel_channels, "libBoardController.so"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


function get_analog_channels(board_id::AnyIntType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:get_analog_channels, "BoardController.dll"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    elseif Sys.isapple()
        ec = ccall((:get_analog_channels, "libBoardController.dylib"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    else
        ec = ccall((:get_analog_channels, "libBoardController.so"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


function get_gyro_channels(board_id::AnyIntType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:get_gyro_channels, "BoardController.dll"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    elseif Sys.isapple()
        ec = ccall((:get_gyro_channels, "libBoardController.dylib"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    else
        ec = ccall((:get_gyro_channels, "libBoardController.so"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


function get_other_channels(board_id::AnyIntType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:get_other_channels, "BoardController.dll"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    elseif Sys.isapple()
        ec = ccall((:get_other_channels, "libBoardController.dylib"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    else
        ec = ccall((:get_other_channels, "libBoardController.so"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


function get_temperature_channels(board_id::AnyIntType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:get_temperature_channels, "BoardController.dll"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    elseif Sys.isapple()
        ec = ccall((:get_temperature_channels, "libBoardController.dylib"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    else
        ec = ccall((:get_temperature_channels, "libBoardController.so"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


function get_resistance_channels(board_id::AnyIntType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:get_resistance_channels, "BoardController.dll"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    elseif Sys.isapple()
        ec = ccall((:get_resistance_channels, "libBoardController.dylib"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    else
        ec = ccall((:get_resistance_channels, "libBoardController.so"), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


function set_log_level(log_level::AnyIntType)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:set_log_level, "BoardController.dll"), Cint, (Cint,), Int32(log_level))
    elseif Sys.isapple()
        ec = ccall((:set_log_level, "libBoardController.dylib"), Cint, (Cint,), Int32(log_level))
    else
        ec = ccall((:set_log_level, "libBoardController.so"), Cint, (Cint,), Int32(log_level))
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in set_log_level ", ec), ec))
    end
end


function enable_board_logger()
    set_log_level(2)
end


function enable_dev_board_logger()
    set_log_level(0)
end


function disable_board_logger()
    set_log_level(6)
end


function log_message(log_level::AnyIntType, message::String)
    ec = STATUS_OK
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


function set_log_file(log_file::String)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:set_log_file, "BoardController.dll"), Cint, (Ptr{UInt8},), log_file)
    elseif Sys.isapple()
        ec = ccall((:set_log_file, "libBoardController.dylib"), Cint, (Ptr{UInt8},), log_file)
    else
        ec = ccall((:set_log_file, "libBoardController.so"), Cint, (Ptr{UInt8},), log_file)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in set_log_file ", ec), ec))
    end
end


struct BoardShim

    master_board_id::Int32
    board_id::Int32
    input_json::String

    function BoardShim(id::AnyIntType, params::BrainFlowInputParams)
        master_id = id
        if id == Integer(STREAMING_BOARD)
            try
                master_id = parse(Int, id)
            catch
                throw(BrainFlowError("for streaming board you need to provide master board id to other_info field"))
            end
        end
        new(master_id, id, JSON.json(params))
    end

end


function prepare_session(board_shim::BoardShim)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:prepare_session, "BoardController.dll"), Cint, (Cint, Ptr{UInt8}), board_shim.board_id, board_shim.input_json)
    elseif Sys.isapple()
        ec = ccall((:prepare_session, "libBoardController.dylib"), Cint, (Cint, Ptr{UInt8}), board_shim.board_id, board_shim.input_json)
    else
        ec = ccall((:prepare_session, "libBoardController.so"), Cint, (Cint, Ptr{UInt8}), board_shim.board_id, board_shim.input_json)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in prepare_session ", ec), ec))
    end
end


function start_stream(board_shim::BoardShim, num_samples::AnyIntType, streamer_params::String)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:start_stream, "BoardController.dll"), Cint, (Cint, Ptr{UInt8}, Cint, Ptr{UInt8}), num_samples, streamer_params,
            board_shim.board_id, board_shim.input_json)
    elseif Sys.isapple()
        ec = ccall((:start_stream, "libBoardController.dylib"), Cint, (Cint, Ptr{UInt8}, Cint, Ptr{UInt8}), num_samples, streamer_params,
            board_shim.board_id, board_shim.input_json)
    else
        ec = ccall((:start_stream, "libBoardController.so"), Cint, (Cint, Ptr{UInt8}, Cint, Ptr{UInt8}), num_samples, streamer_params,
            board_shim.board_id, board_shim.input_json)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in start_stream ", ec), ec))
    end
end


function start_stream(board_shim::BoardShim)
    start_stream(board_shim, 45000, "")
end


function is_prepared(board_shim::BoardShim)
    val = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:is_prepared, "BoardController.dll"), Cint, (Ptr{Cint}, Cint, Ptr{UInt8}), val, board_shim.board_id, board_shim.input_json)
    elseif Sys.isapple()
        ec = ccall((:is_prepared, "libBoardController.dylib"), Cint, (Ptr{Cint}, Cint, Ptr{UInt8}), val, board_shim.board_id, board_shim.input_json)
    else
        ec = ccall((:is_prepared, "libBoardController.so"), Cint, (Ptr{Cint}, Cint, Ptr{UInt8}), val, board_shim.board_id, board_shim.input_json)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in is_prepared ", ec), ec))
    end
    value = Bool(val[1])
    value
end


function get_board_data_count(board_shim::BoardShim)
    val = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:get_board_data_count, "BoardController.dll"), Cint, (Ptr{Cint}, Cint, Ptr{UInt8}), val, board_shim.board_id, board_shim.input_json)
    elseif Sys.isapple()
        ec = ccall((:get_board_data_count, "libBoardController.dylib"), Cint, (Ptr{Cint}, Cint, Ptr{UInt8}), val, board_shim.board_id, board_shim.input_json)
    else
        ec = ccall((:get_board_data_count, "libBoardController.so"), Cint, (Ptr{Cint}, Cint, Ptr{UInt8}), val, board_shim.board_id, board_shim.input_json)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get_board_data_count ", ec), ec))
    end
    value = val[1]
    value
end


function stop_stream(board_shim::BoardShim)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:stop_stream, "BoardController.dll"), Cint, (Cint, Ptr{UInt8}), board_shim.board_id, board_shim.input_json)
    elseif Sys.isapple()
        ec = ccall((:stop_stream, "libBoardController.dylib"), Cint, (Cint, Ptr{UInt8}), board_shim.board_id, board_shim.input_json)
    else
        ec = ccall((:stop_stream, "libBoardController.so"), Cint, (Cint, Ptr{UInt8}), board_shim.board_id, board_shim.input_json)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in stop_stream ", ec), ec))
    end
end


function release_session(board_shim::BoardShim)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:release_session, "BoardController.dll"), Cint, (Cint, Ptr{UInt8}), board_shim.board_id, board_shim.input_json)
    elseif Sys.isapple()
        ec = ccall((:release_session, "libBoardController.dylib"), Cint, (Cint, Ptr{UInt8}), board_shim.board_id, board_shim.input_json)
    else
        ec = ccall((:release_session, "libBoardController.so"), Cint, (Cint, Ptr{UInt8}), board_shim.board_id, board_shim.input_json)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in release_session ", ec), ec))
    end
end


function get_board_data(board_shim::BoardShim)
    data_size = get_board_data_count(board_shim)
    num_rows = get_num_rows(board_shim.master_board_id)
    val = Vector{Float64}(undef, num_rows * data_size)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:get_board_data, "BoardController.dll"), Cint, (Cint, Ptr{Float64}, Cint, Ptr{UInt8}), 
            data_size, val, board_shim.board_id, board_shim.input_json)
    elseif Sys.isapple()
        ec = ccall((:get_board_data, "libBoardController.dylib"), Cint, (Cint, Ptr{Float64}, Cint, Ptr{UInt8}),
            data_size, val, board_shim.board_id, board_shim.input_json)
    else
        ec = ccall((:get_board_data, "libBoardController.so"), Cint, (Cint, Ptr{Float64}, Cint, Ptr{UInt8}),
            data_size, val, board_shim.board_id, board_shim.input_json)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get_board_data ", ec), ec))
    end
    value = transpose(reshape(val, (data_size, num_rows)))
    value
end


function get_current_board_data(num_samples::AnyIntType, board_shim::BoardShim)
    data_size = Vector{Cint}(undef, 1)
    num_rows = get_num_rows(board_shim.master_board_id)
    val = Vector{Float64}(undef, num_rows * num_samples)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:get_current_board_data, "BoardController.dll"), Cint, (Cint, Ptr{Float64}, Ptr{Cint}, Cint, Ptr{UInt8}), 
            num_samples, val, data_size, board_shim.board_id, board_shim.input_json)
    elseif Sys.isapple()
        ec = ccall((:get_current_board_data, "libBoardController.dylib"), Cint, (Cint, Ptr{Float64}, Ptr{Cint}, Cint, Ptr{UInt8}),
            num_samples, val, data_size, board_shim.board_id, board_shim.input_json)
    else
        ec = ccall((:get_current_board_data, "libBoardController.so"), Cint, (Cint, Ptr{Float64}, Ptr{Cint}, Cint, Ptr{UInt8}),
            num_samples, val, data_size, board_shim.board_id, board_shim.input_json)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get_current_board_data ", ec), ec))
    end
    value = transpose(reshape(val[1:data_size[1] * num_rows], (data_size[1], num_rows)))
    value
end

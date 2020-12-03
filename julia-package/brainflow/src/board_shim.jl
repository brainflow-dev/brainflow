import JSON

@enum BoardIds begin

    PLAYBACK_FILE_BOARD = -3
    STREAMING_BOARD = -2
    SYNTHETIC_BOARD = -1
    CYTON_BOARD = 0
    GANGLION_BOARD = 1
    CYTON_DAISY_BOARD = 2
    GALEA_BOARD = 3
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
    FREEEEG32_BOARD = 17

end

BoardIdType = Union{BoardIds, Integer}

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
    file::String

    function BrainFlowInputParams()
        new("", "", "", 0, Integer(NONE), "", 0, "", "")
    end

end


function get_timestamp_channel(board_id::BoardIdType)
    channel = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    ec = ccall((:get_timestamp_channel, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{Cint}), Int32(board_id), channel)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channel[1] + 1
    value
end


function get_package_num_channel(board_id::BoardIdType)
    channel = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    ec = ccall((:get_package_num_channel, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{Cint}), Int32(board_id), channel)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channel[1] + 1
    value
end


function get_battery_channel(board_id::BoardIdType)
    channel = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    ec = ccall((:get_battery_channel, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{Cint}), Int32(board_id), channel)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channel[1] + 1
    value
end


function get_sampling_rate(board_id::BoardIdType)
    val = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    ec = ccall((:get_sampling_rate, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{Cint}), Int32(board_id), val)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    value = val[1]
    value
end


function get_num_rows(board_id::BoardIdType)
    val = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    ec = ccall((:get_num_rows, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{Cint}), Int32(board_id), val)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # here dont need to add 1, last element included
    value = val[1]
    value
end


function get_eeg_names(board_id::BoardIdType)
    names_string = Vector{Cuchar}(undef, 4096)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    ec = ccall((:get_eeg_names, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{UInt8}, Ptr{Cint}), Int32(board_id), names_string, len)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    sub_string = String(names_string)[1:len[1]]
    value = split(sub_string, ',')
    value
end


function get_eeg_channels(board_id::BoardIdType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    ec = ccall((:get_eeg_channels, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


function get_exg_channels(board_id::BoardIdType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    ec = ccall((:get_exg_channels, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


function get_emg_channels(board_id::BoardIdType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    ec = ccall((:get_emg_channels, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


function get_ecg_channels(board_id::BoardIdType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    ec = ccall((:get_ecg_channels, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


function get_eog_channels(board_id::BoardIdType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    ec = ccall((:get_eog_channels, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


function get_eda_channels(board_id::BoardIdType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    ec = ccall((:get_eda_channels, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


function get_ppg_channels(board_id::BoardIdType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    ec = ccall((:get_ppg_channels, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


function get_accel_channels(board_id::BoardIdType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    ec = ccall((:get_accel_channels, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


function get_analog_channels(board_id::BoardIdType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
    ec = ccall((:get_analog_channels, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


function get_gyro_channels(board_id::BoardIdType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
        ec = ccall((:get_gyro_channels, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


function get_other_channels(board_id::BoardIdType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
        ec = ccall((:get_other_channels, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


function get_temperature_channels(board_id::BoardIdType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
        ec = ccall((:get_temperature_channels, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


function get_resistance_channels(board_id::BoardIdType)
    channels = Vector{Cint}(undef, 512)
    len = Vector{Cint}(undef, 1)
    ec = STATUS_OK
        ec = ccall((:get_resistance_channels, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{Cint}, Ptr{Cint}), Int32(board_id), channels, len)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get board info ", ec), ec))
    end
    # julia counts from 1
    value = channels[1:len[1]] .+ 1
    value
end


struct BoardShim

    master_board_id::Int32
    board_id::Int32
    input_json::String

    function BoardShim(id::Integer, params::BrainFlowInputParams)
        master_id = id
        if id == Integer(STREAMING_BOARD) || id == Integer(PLAYBACK_FILE_BOARD)
            try
                master_id = parse(Int, id)
            catch
                throw(BrainFlowError("you need to provide master board id to other_info field", 1))
            end
        end
        new(master_id, id, JSON.json(params))
    end

end
BoardShim(id::BoardIds, params::BrainFlowInputParams) = BoardShim(Integer(id), params)
BoardShim(id) = BoardShim(id, BrainFlowInputParams())

function prepare_session(board_shim::BoardShim)
    ec = STATUS_OK
        ec = ccall((:prepare_session, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{UInt8}), board_shim.board_id, board_shim.input_json)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in prepare_session ", ec), ec))
    end
end


function start_stream(board_shim::BoardShim, num_samples::Int, streamer_params::String)
    ec = STATUS_OK
        ec = ccall((:start_stream, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{UInt8}, Cint, Ptr{UInt8}), num_samples, streamer_params,
            board_shim.board_id, board_shim.input_json)
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
        ec = ccall((:is_prepared, BOARD_CONTROLLER_INTERFACE), Cint, (Ptr{Cint}, Cint, Ptr{UInt8}), val, board_shim.board_id, board_shim.input_json)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in is_prepared ", ec), ec))
    end
    value = Bool(val[1])
    value
end


function get_board_data_count(board_shim::BoardShim)
    val = Vector{Cint}(undef, 1)
    ec = STATUS_OK
        ec = ccall((:get_board_data_count, BOARD_CONTROLLER_INTERFACE), Cint, (Ptr{Cint}, Cint, Ptr{UInt8}), val, board_shim.board_id, board_shim.input_json)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get_board_data_count ", ec), ec))
    end
    value = val[1]
    value
end


function stop_stream(board_shim::BoardShim)
    ec = STATUS_OK
        ec = ccall((:stop_stream, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{UInt8}), board_shim.board_id, board_shim.input_json)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in stop_stream ", ec), ec))
    end
end


function release_session(board_shim::BoardShim)
    ec = STATUS_OK
        ec = ccall((:release_session, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{UInt8}), board_shim.board_id, board_shim.input_json)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in release_session ", ec), ec))
    end
end

function config_board(config::String, board_shim::BoardShim)
    ec = STATUS_OK
    resp_string = Vector{Cuchar}(undef, 4096)
    len = Vector{Cint}(undef, 1)
        ec = ccall((:config_board, BOARD_CONTROLLER_INTERFACE), Cint, (Ptr{UInt8}, Ptr{UInt8}, Ptr{Cint}, Cint, Ptr{UInt8}),
            config, resp_string, len, board_shim.board_id, board_shim.input_json)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in config_board ", ec), ec))
    end
    sub_string = String(resp_string)[1:len[1]]
    sub_string
end

function get_board_data(board_shim::BoardShim)
    data_size = get_board_data_count(board_shim)
    num_rows = get_num_rows(board_shim.master_board_id)
    val = Vector{Float64}(undef, num_rows * data_size)
    ec = STATUS_OK
        ec = ccall((:get_board_data, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{Float64}, Cint, Ptr{UInt8}), 
            data_size, val, board_shim.board_id, board_shim.input_json)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get_board_data ", ec), ec))
    end
    value = transpose(reshape(val, (data_size, num_rows)))
    value
end


function get_current_board_data(num_samples::Integer, board_shim::BoardShim)
    data_size = Vector{Cint}(undef, 1)
    num_rows = get_num_rows(board_shim.master_board_id)
    val = Vector{Float64}(undef, num_rows * num_samples)
    ec = STATUS_OK
        ec = ccall((:get_current_board_data, BOARD_CONTROLLER_INTERFACE), Cint, (Cint, Ptr{Float64}, Ptr{Cint}, Cint, Ptr{UInt8}), 
            num_samples, val, data_size, board_shim.board_id, board_shim.input_json)
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in get_current_board_data ", ec), ec))
    end
    value = transpose(reshape(val[1:data_size[1] * num_rows], (data_size[1], num_rows)))
    value
end

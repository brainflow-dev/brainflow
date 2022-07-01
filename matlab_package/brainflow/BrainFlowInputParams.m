classdef BrainFlowInputParams
    % BrainFlow input params, check docs for params for your device
    properties
        serial_port
        mac_address
        ip_address
        ip_port
        ip_protocol
        other_info
        timeout
        serial_number
        file
        master_board
        preset
    end
    methods
        function obj = BrainFlowInputParams()
            obj.serial_port = '';
            obj.mac_address = '';
            obj.ip_address = '';
            obj.ip_port = 0;
            obj.ip_protocol = int32(IpProtocolTypes.NO_IP_PROTOCOL);
            obj.other_info = '';
            obj.timeout = 0;
            obj.serial_number = '';
            obj.file = '';
            obj.master_board = int32(BoardIds.NO_BOARD);
            obj.preset = int32(BrainFlowPresets.DEFAULT_PRESET);
        end
        function json_string = to_json(obj)
            json_string = jsonencode(obj);
        end
    end
end
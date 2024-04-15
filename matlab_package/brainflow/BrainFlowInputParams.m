classdef BrainFlowInputParams
    % BrainFlow input params, check docs for params for your device
    properties
        serial_port
        mac_address
        ip_address
        ip_address_aux
        ip_address_anc
        ip_port
        ip_port_aux
        ip_port_anc
        ip_protocol
        other_info
        timeout
        serial_number
        file
        file_aux
        file_anc
        master_board
    end
    methods
        function obj = BrainFlowInputParams()
            obj.serial_port = '';
            obj.mac_address = '';
            obj.ip_address = '';
            obj.ip_address_aux = '';
            obj.ip_address_anc = '';
            obj.ip_port = 0;
            obj.ip_port_aux = 0;
            obj.ip_port_anc = 0;
            obj.ip_protocol = int32(IpProtocolTypes.NO_IP_PROTOCOL);
            obj.other_info = '';
            obj.timeout = 0;
            obj.serial_number = '';
            obj.file = '';
            obj.file_aux = '';
            obj.file_anc = '';
            obj.master_board = int32(BoardIds.NO_BOARD);
        end
        function json_string = to_json(obj)
            json_string = jsonencode(obj);
        end
    end
end
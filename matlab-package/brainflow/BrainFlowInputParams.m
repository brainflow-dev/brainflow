classdef BrainFlowInputParams
    properties
        serial_port
        mac_address
        ip_address
        ip_port
        ip_protocol
        other_info
        timeout
    end
    methods
        function obj = BrainFlowInputParams ()
            obj.serial_port = '';
            obj.mac_address = '';
            obj.ip_address = '';
            obj.ip_port = 0;
            obj.ip_protocol = int32 (IpProtocolType.NONE);
            obj.other_info = '';
            obj.timeout = 0;
        end
        function json_string = to_json (obj)
            json_string = jsonencode(obj);
        end
    end
end
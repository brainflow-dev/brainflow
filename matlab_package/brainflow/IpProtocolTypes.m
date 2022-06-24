classdef IpProtocolTypes < int32
    % Store all possible IP protocols
    enumeration
        NO_IP_PROTOCOL(0)
        UDP(1)
        TCP(2)
    end
end
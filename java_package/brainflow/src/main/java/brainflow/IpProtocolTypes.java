package brainflow;

import java.util.HashMap;
import java.util.Map;

public enum IpProtocolTypes
{

    NO_IP_PROTOCOL (0),
    UDP (1),
    TCP (2);

    private final int protocol;
    private static final Map<Integer, IpProtocolTypes> ip_map = new HashMap<Integer, IpProtocolTypes> ();

    public int get_code ()
    {
        return protocol;
    }

    public static String string_from_code (final int code)
    {
        return from_code (code).name ();
    }

    public static IpProtocolTypes from_code (final int code)
    {
        final IpProtocolTypes element = ip_map.get (code);
        return element;
    }

    IpProtocolTypes (final int code)
    {
        protocol = code;
    }

    static
    {
        for (final IpProtocolTypes ec : IpProtocolTypes.values ())
        {
            ip_map.put (ec.get_code (), ec);
        }
    }

}

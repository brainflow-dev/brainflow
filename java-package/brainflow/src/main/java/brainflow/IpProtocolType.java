package brainflow;

import java.util.HashMap;
import java.util.Map;

public enum IpProtocolType
{

    NONE (0),
    UDP (1),
    TCP (2);

    private final int protocol;
    private static final Map<Integer, IpProtocolType> ip_map = new HashMap<Integer, IpProtocolType> ();

    public int get_code ()
    {
        return protocol;
    }

    public static String string_from_code (final int code)
    {
        return from_code (code).name ();
    }

    public static IpProtocolType from_code (final int code)
    {
        final IpProtocolType element = ip_map.get (code);
        return element;
    }

    IpProtocolType (final int code)
    {
        protocol = code;
    }

    static
    {
        for (final IpProtocolType ec : IpProtocolType.values ())
        {
            ip_map.put (ec.get_code (), ec);
        }
    }

}

package brainflow;

import java.util.HashMap;
import java.util.Map;

/**
 * enum to store all supported noise types
 */
public enum NoiseTypes
{

    FIFTY (0),
    SIXTY (1),
    EACH (2);

    private final int noise_type;
    private static final Map<Integer, NoiseTypes> nt_map = new HashMap<Integer, NoiseTypes> ();

    public int get_code ()
    {
        return noise_type;
    }

    public static String string_from_code (final int code)
    {
        return from_code (code).name ();
    }

    public static NoiseTypes from_code (final int code)
    {
        final NoiseTypes element = nt_map.get (code);
        return element;
    }

    NoiseTypes (final int code)
    {
        noise_type = code;
    }

    static
    {
        for (final NoiseTypes ao : NoiseTypes.values ())
        {
            nt_map.put (ao.get_code (), ao);
        }
    }
}

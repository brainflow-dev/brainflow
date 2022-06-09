package brainflow;

import java.util.HashMap;
import java.util.Map;

/**
 * enum to store all possible Wavelet Extenstion Types
 */
public enum WaveletExtensionTypes
{

    SYMMETRIC (0),
    PERIODIC (1);

    private final int value;
    private static final Map<Integer, WaveletExtensionTypes> map = new HashMap<Integer, WaveletExtensionTypes> ();

    public int get_code ()
    {
        return value;
    }

    public static String string_from_code (final int code)
    {
        return from_code (code).name ();
    }

    public static WaveletExtensionTypes from_code (final int code)
    {
        final WaveletExtensionTypes element = map.get (code);
        return element;
    }

    WaveletExtensionTypes (final int code)
    {
        value = code;
    }

    static
    {
        for (final WaveletExtensionTypes type : WaveletExtensionTypes.values ())
        {
            map.put (type.get_code (), type);
        }
    }
}

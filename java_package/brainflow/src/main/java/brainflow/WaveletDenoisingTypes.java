package brainflow;

import java.util.HashMap;
import java.util.Map;

/**
 * enum to store all possible Wavelet Denoising Types
 */
public enum WaveletDenoisingTypes
{

    VISUSHRINK (0),
    SURESHRINK (1);

    private final int value;
    private static final Map<Integer, WaveletDenoisingTypes> map = new HashMap<Integer, WaveletDenoisingTypes> ();

    public int get_code ()
    {
        return value;
    }

    public static String string_from_code (final int code)
    {
        return from_code (code).name ();
    }

    public static WaveletDenoisingTypes from_code (final int code)
    {
        final WaveletDenoisingTypes element = map.get (code);
        return element;
    }

    WaveletDenoisingTypes (final int code)
    {
        value = code;
    }

    static
    {
        for (final WaveletDenoisingTypes type : WaveletDenoisingTypes.values ())
        {
            map.put (type.get_code (), type);
        }
    }
}

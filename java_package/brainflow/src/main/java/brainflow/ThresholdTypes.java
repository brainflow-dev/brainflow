package brainflow;

import java.util.HashMap;
import java.util.Map;

/**
 * enum to store all possible Threshold Types
 */
public enum ThresholdTypes
{

    SOFT (0),
    HARD (1);

    private final int value;
    private static final Map<Integer, ThresholdTypes> map = new HashMap<Integer, ThresholdTypes> ();

    public int get_code ()
    {
        return value;
    }

    public static String string_from_code (final int code)
    {
        return from_code (code).name ();
    }

    public static ThresholdTypes from_code (final int code)
    {
        final ThresholdTypes element = map.get (code);
        return element;
    }

    ThresholdTypes (final int code)
    {
        value = code;
    }

    static
    {
        for (final ThresholdTypes type : ThresholdTypes.values ())
        {
            map.put (type.get_code (), type);
        }
    }
}

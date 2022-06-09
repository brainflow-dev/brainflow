package brainflow;

import java.util.HashMap;
import java.util.Map;

/**
 * enum to store all possible Noise Level Types
 */
public enum NoiseEstimationLevelTypes
{

    FIRST_LEVEL (0),
    ALL_LEVELS (1);

    private final int value;
    private static final Map<Integer, NoiseEstimationLevelTypes> map = new HashMap<Integer, NoiseEstimationLevelTypes> ();

    public int get_code ()
    {
        return value;
    }

    public static String string_from_code (final int code)
    {
        return from_code (code).name ();
    }

    public static NoiseEstimationLevelTypes from_code (final int code)
    {
        final NoiseEstimationLevelTypes element = map.get (code);
        return element;
    }

    NoiseEstimationLevelTypes (final int code)
    {
        value = code;
    }

    static
    {
        for (final NoiseEstimationLevelTypes type : NoiseEstimationLevelTypes.values ())
        {
            map.put (type.get_code (), type);
        }
    }
}

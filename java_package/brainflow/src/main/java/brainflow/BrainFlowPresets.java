package brainflow;

import java.util.HashMap;
import java.util.Map;

/**
 * enum to store all supported presets
 */
public enum BrainFlowPresets
{

    DEFAULT_PRESET (0),
    AUXILIARY_PRESET (1),
    ANCILLARY_PRESET (2);

    private final int preset;
    private static final Map<Integer, BrainFlowPresets> my_map = new HashMap<Integer, BrainFlowPresets> ();

    public int get_code ()
    {
        return preset;
    }

    public static String string_from_code (final int code)
    {
        return from_code (code).name ();
    }

    public static BrainFlowPresets from_code (final int code)
    {
        final BrainFlowPresets element = my_map.get (code);
        return element;
    }

    BrainFlowPresets (final int code)
    {
        preset = code;
    }

    static
    {
        for (final BrainFlowPresets pr : BrainFlowPresets.values ())
        {
            my_map.put (pr.get_code (), pr);
        }
    }
}

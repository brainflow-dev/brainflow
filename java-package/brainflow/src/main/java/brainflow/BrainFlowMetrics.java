package brainflow;

import java.util.HashMap;
import java.util.Map;

public enum BrainFlowMetrics
{

    RELAXATION (0),
    CONCENTRATION (1),
    USER_DEFINED (2);

    private final int protocol;
    private static final Map<Integer, BrainFlowMetrics> metr_map = new HashMap<Integer, BrainFlowMetrics> ();

    public int get_code ()
    {
        return protocol;
    }

    public static String string_from_code (final int code)
    {
        return from_code (code).name ();
    }

    public static BrainFlowMetrics from_code (final int code)
    {
        final BrainFlowMetrics element = metr_map.get (code);
        return element;
    }

    BrainFlowMetrics (final int code)
    {
        protocol = code;
    }

    static
    {
        for (final BrainFlowMetrics ec : BrainFlowMetrics.values ())
        {
            metr_map.put (ec.get_code (), ec);
        }
    }

}

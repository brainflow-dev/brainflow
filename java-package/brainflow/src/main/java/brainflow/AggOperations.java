package brainflow;

import java.util.HashMap;
import java.util.Map;

/**
 * enum to store all supported aggregation operations
 */
public enum AggOperations
{

    MEAN (0),
    MEDIAN (1),
    EACH (2);

    private final int agg_operation;
    private static final Map<Integer, AggOperations> ao_map = new HashMap<Integer, AggOperations> ();

    public int get_code ()
    {
        return agg_operation;
    }

    public static String string_from_code (final int code)
    {
        return from_code (code).name ();
    }

    public static AggOperations from_code (final int code)
    {
        final AggOperations element = ao_map.get (code);
        return element;
    }

    AggOperations (final int code)
    {
        agg_operation = code;
    }

    static
    {
        for (final AggOperations ao : AggOperations.values ())
        {
            ao_map.put (ao.get_code (), ao);
        }
    }
}

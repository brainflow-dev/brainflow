package brainflow;

import java.util.HashMap;
import java.util.Map;

/**
 * enum to store all supported detrend operations
 */
public enum DetrendOperations
{

    NONE (0),
    CONSTANT (1),
    LINEAR (2);

    private final int detrend_operation;
    private static final Map<Integer, DetrendOperations> dt_map = new HashMap<Integer, DetrendOperations> ();

    public int get_code ()
    {
        return detrend_operation;
    }

    public static String string_from_code (final int code)
    {
        return from_code (code).name ();
    }

    public static DetrendOperations from_code (final int code)
    {
        final DetrendOperations element = dt_map.get (code);
        return element;
    }

    DetrendOperations (final int code)
    {
        detrend_operation = code;
    }

    static
    {
        for (final DetrendOperations dto : DetrendOperations.values ())
        {
            dt_map.put (dto.get_code (), dto);
        }
    }
}

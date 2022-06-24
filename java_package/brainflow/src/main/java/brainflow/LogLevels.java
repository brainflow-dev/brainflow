package brainflow;

import java.util.HashMap;
import java.util.Map;

public enum LogLevels
{

    LEVEL_TRACE (0),
    LEVEL_DEBUG (1),
    LEVEL_INFO (2),
    LEVEL_WARN (3),
    LEVEL_ERROR (4),
    LEVEL_CRITICAL (5),
    LEVEL_OFF (6);

    private final int log_level;
    private static final Map<Integer, LogLevels> ll_map = new HashMap<Integer, LogLevels> ();

    public int get_code ()
    {
        return log_level;
    }

    public static String string_from_code (final int code)
    {
        return from_code (code).name ();
    }

    public static LogLevels from_code (final int code)
    {
        final LogLevels element = ll_map.get (code);
        return element;
    }

    LogLevels (final int code)
    {
        log_level = code;
    }

    static
    {
        for (final LogLevels ll : LogLevels.values ())
        {
            ll_map.put (ll.get_code (), ll);
        }
    }
}

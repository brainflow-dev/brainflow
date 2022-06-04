package brainflow;

import java.util.HashMap;
import java.util.Map;

public enum WindowOperations
{

    NO_WINDOW (0),
    HANNING (1),
    HAMMING (2),
    BLACKMAN_HARRIS (3);

    private final int window;
    private static final Map<Integer, WindowOperations> window_map = new HashMap<Integer, WindowOperations> ();

    public int get_code ()
    {
        return window;
    }

    public static String string_from_code (final int code)
    {
        return from_code (code).name ();
    }

    public static WindowOperations from_code (final int code)
    {
        final WindowOperations element = window_map.get (code);
        return element;
    }

    WindowOperations (final int code)
    {
        window = code;
    }

    static
    {
        for (final WindowOperations win : WindowOperations.values ())
        {
            window_map.put (win.get_code (), win);
        }
    }
}

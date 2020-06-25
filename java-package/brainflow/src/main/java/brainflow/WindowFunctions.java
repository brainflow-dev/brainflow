package brainflow;

import java.util.HashMap;
import java.util.Map;

public enum WindowFunctions
{

    NO_WINDOW (0),
    HANNING (1),
    HAMMING (2),
    BLACKMAN_HARRIS (3);

    private final int window;
    private static final Map<Integer, WindowFunctions> window_map = new HashMap<Integer, WindowFunctions> ();

    public int get_code ()
    {
        return window;
    }

    public static String string_from_code (final int code)
    {
        return from_code (code).name ();
    }

    public static WindowFunctions from_code (final int code)
    {
        final WindowFunctions element = window_map.get (code);
        return element;
    }

    WindowFunctions (final int code)
    {
        window = code;
    }

    static
    {
        for (final WindowFunctions win : WindowFunctions.values ())
        {
            window_map.put (win.get_code (), win);
        }
    }
}

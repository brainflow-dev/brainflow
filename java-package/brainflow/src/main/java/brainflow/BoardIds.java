package brainflow;

import java.util.HashMap;
import java.util.Map;

/**
 * enum to store all supported boards
 */
public enum BoardIds
{
    STREAMING_BOARD (-2),
    SYNTHETIC_BOARD (-1),
    CYTON_BOARD (0),
    GANGLION_BOARD (1),
    CYTON_DAISY_BOARD (2),
    NOVAXR_BOARD (3),
    GANGLION_WIFI_BOARD (4),
    CYTON_WIFI_BOARD (5),
    CYTON_DAISY_WIFI_BOARD (6),
    BRAINBIT_BOARD (7);

    private final int board_id;
    private static final Map<Integer, BoardIds> bi_map = new HashMap<Integer, BoardIds> ();

    public int get_code ()
    {
        return board_id;
    }

    public static String string_from_code (final int code)
    {
        return from_code (code).name ();
    }

    public static BoardIds from_code (final int code)
    {
        final BoardIds element = bi_map.get (code);
        return element;
    }

    BoardIds (final int code)
    {
        board_id = code;
    }

    static
    {
        for (final BoardIds bi : BoardIds.values ())
        {
            bi_map.put (bi.get_code (), bi);
        }
    }
}

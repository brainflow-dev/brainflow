package brainflow;

import java.util.HashMap;
import java.util.Map;

public enum BoardIds {

    SYNTHETIC_BOARD (-1),
    CYTON_BOARD (0),
    GANGLION_BOARD (1),
    CYTON_DAISY_BOARD (2),
    NOVAXR_BOARD (3),
    GANGLION_WIFI_BOARD (4),
    CYTON_WIFI_BOARD (5),
    CYTON_DAISY_WIFI_BOARD (6);
    
    private final int exit_code;
    private static final Map<Integer, ExitCode> ec_map = new HashMap<Integer, ExitCode>();
    
    public int get_code () {
        return exit_code;
    }
    
    public static String string_from_code (final int code) {
        return from_code (code).name ();
    }
 
    public static ExitCode from_code (final int code) {
        final ExitCode element = ec_map.get (code);
        return element;
    }
 
    BoardIds(final int code) {
        exit_code = code;
    }

    static {
        for (final ExitCode ec : ExitCode.values ()) {
            ec_map.put(ec.get_code(), ec);
        }
    }
}

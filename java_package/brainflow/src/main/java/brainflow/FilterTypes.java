package brainflow;

import java.util.HashMap;
import java.util.Map;

/**
 * enum to store all possible filter types
 */
public enum FilterTypes
{

    BUTTERWORTH (0),
    CHEBYSHEV_TYPE_1 (1),
    BESSEL (2);

    private final int filter_type;
    private static final Map<Integer, FilterTypes> ft_map = new HashMap<Integer, FilterTypes> ();

    public int get_code ()
    {
        return filter_type;
    }

    public static String string_from_code (final int code)
    {
        return from_code (code).name ();
    }

    public static FilterTypes from_code (final int code)
    {
        final FilterTypes element = ft_map.get (code);
        return element;
    }

    FilterTypes (final int code)
    {
        filter_type = code;
    }

    static
    {
        for (final FilterTypes ft : FilterTypes.values ())
        {
            ft_map.put (ft.get_code (), ft);
        }
    }
}

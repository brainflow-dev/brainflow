package brainflow;

import java.util.HashMap;
import java.util.Map;

public enum BrainFlowClassifiers
{

    REGRESSION (0),
    KNN (1),
    SVM (2),
    LDA (3),
    DYN_LIB_CLASSIFIER (4);

    private final int protocol;
    private static final Map<Integer, BrainFlowClassifiers> cl_map = new HashMap<Integer, BrainFlowClassifiers> ();

    public int get_code ()
    {
        return protocol;
    }

    public static String string_from_code (final int code)
    {
        return from_code (code).name ();
    }

    public static BrainFlowClassifiers from_code (final int code)
    {
        final BrainFlowClassifiers element = cl_map.get (code);
        return element;
    }

    BrainFlowClassifiers (final int code)
    {
        protocol = code;
    }

    static
    {
        for (final BrainFlowClassifiers ec : BrainFlowClassifiers.values ())
        {
            cl_map.put (ec.get_code (), ec);
        }
    }

}

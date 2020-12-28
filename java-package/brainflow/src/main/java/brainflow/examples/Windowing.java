package brainflow.examples;

import java.util.Arrays;

import brainflow.DataFilter;

public class Windowing
{

    public static void main (String[] args) throws Exception
    {
        int window_len = 20;
        for (int window_function = 0; window_function < 4; window_function++)
        {
            double[] window_data = DataFilter.get_window (window_function, window_len);
            System.out.println ("Window data:");
            System.out.println (Arrays.toString (window_data));
        }
    }
}

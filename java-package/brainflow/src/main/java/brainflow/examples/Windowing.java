package brainflow.examples;

import java.util.Arrays;

public class Windowing
{

    public static void main (String[] args) throws Exception
    {
        int window_len = 20;
        for (int window_id = 0; i < 4; i++)
        {
            double[] window_data = DataFilter.get_window (window_id, window_len);
            System.out.println ("Window data:");
            System.out.println (Arrays.toString (window_data));
        }
    }
}

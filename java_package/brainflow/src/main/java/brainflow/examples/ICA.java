package brainflow.examples;

import java.util.List;
import brainflow.DataFilter;

public class ICA
{
    public static void main (String[] args) throws Exception
    {
        // Two simultaneous mixtures: rows are channels and columns are samples.
        int samples = 1024;
        double[][] data = new double[2][samples];
        for (int i = 0; i < samples; i++)
        {
            double t = i / 256.0;
            double first = Math.sin (2.0 * Math.PI * 7.0 * t);
            double second = Math.pow (Math.sin (2.0 * Math.PI * 13.0 * t), 3);
            data[0][i] = first + 0.3 * second;
            data[1][i] = 0.2 * first + second;
        }
        List<double[][]> ica = DataFilter.perform_ica (data, 2);
        // Component order and sign are arbitrary.
        System.out.println ("Recovered " + ica.get (3).length + " sources from " + samples + " samples");
    }
}

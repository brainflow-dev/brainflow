package brainflow.examples;

import java.util.Arrays;

import org.apache.commons.lang3.tuple.Pair;

import brainflow.BoardShim;
import brainflow.DataFilter;
import brainflow.MLModel;

public class CSP
{

    public static void main (String[] args) throws Exception
    {
        System.out.println (BoardShim.get_version ());
        System.out.println (DataFilter.get_version ());
        System.out.println (MLModel.get_version ());

        int n_channels = 2;
        double[] labels =
        { 0, 1 };
        double[][][] data =
        {
                {
                        { 6, 3, 1, 5 },
                        { 3, 0, 5, 1 } },
                {
                        { 1, 5, 6, 2 },
                        { 5, 1, 2, 2 } } };

        Pair<double[][], double[]> csp_data = DataFilter.get_csp (data, labels);

        System.out.print ("filters = \n");

        for (int i = 0; i < n_channels; i++)
        {
            System.out.println (Arrays.toString (csp_data.getLeft ()[i]));
        }
        System.out.println ();
    }
}

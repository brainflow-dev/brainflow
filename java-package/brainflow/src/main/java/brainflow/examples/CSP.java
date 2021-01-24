package brainflow.examples;

import java.io.InputStream;
import java.util.Arrays;

import org.apache.commons.lang3.SystemUtils;
import org.apache.commons.lang3.tuple.MutablePair;
import org.apache.commons.lang3.tuple.Pair;

import brainflow.DataFilter;

public class CSP
{

    public static void main (String[] args) throws Exception
    {
        double[] labels = {0, 1};
        double[][][] data = {
                            {{6, 3, 1, 5}, {3, 0, 5, 1}},
                            {{1, 5, 6, 2}, {5, 1, 2, 2}}
                         };
        Pair<double[][], double[]> csp_data = DataFilter.DataFilter.get_csp(data, labels);
        System.out.print ("filters = \n");
            for (int i = 0; i < 2; i++)
            {
                for (int j = 0; j < 2; j++) 
                {
                    System.out.print (csp_data.getLeft()[i][j] + " ");
                }
            }
            System.out.println ();
    }
}

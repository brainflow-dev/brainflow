using System;
using brainflow;

namespace test
{
    class Csp
    {
        static void Main (string[] args)
        {
            // for demo get the CSP filters
            int n_epochs = 2;
            int n_channels = 2;
            int n_times = 4;

            double[,,] data = new double[2, 2, 4] {{{6, 3, 1, 5}, {3, 0, 5, 1}}, {{1, 5, 6, 2}, {5, 1, 2, 2}}};
            double[] labels = new double[2] {0, 1};

            Tuple<double[,], double[]> csp = DataFilter.get_csp(data, labels);

            Console.WriteLine("filters = ");
            for (int i = 0; i < n_channels; i++)
            {
                Console.WriteLine();
                for (int j = 0; j < n_channels; j++)
                {
                    Console.Write(csp.Item1[i, j] + "  ");
                }
            }
        }
    }
}

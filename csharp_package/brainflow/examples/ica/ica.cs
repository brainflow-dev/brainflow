using System;
using brainflow;

namespace examples
{
    class ICA
    {
        static void Main (string[] args)
        {
            // Two simultaneous mixtures: rows are channels and columns are samples.
            const int samples = 1024;
            double[,] data = new double[2, samples];
            for (int i = 0; i < samples; i++)
            {
                double t = i / 256.0;
                double first = Math.Sin (2.0 * Math.PI * 7.0 * t);
                double second = Math.Pow (Math.Sin (2.0 * Math.PI * 13.0 * t), 3);
                data[0, i] = first + 0.3 * second;
                data[1, i] = 0.2 * first + second;
            }
            var ica = DataFilter.perform_ica (data, 2);
            // Component order and sign are arbitrary.
            Console.WriteLine ("Recovered " + ica.Item4.GetLength (0) + " sources from " + samples + " samples");
        }
    }
}

using System;
using brainflow;

class Program
{
    static void Reject(Action action)
    {
        try { action(); }
        catch (BrainFlowError) { return; }
        throw new Exception("Expected invalid input to be rejected");
    }

    static void CheckMLSnapshot(string modelPath)
    {
        var parameters = new BrainFlowModelParams(2, 2);
        parameters.file = System.IO.Path.GetFullPath(modelPath);
        parameters.max_array_size = 2;
        var model = new MLModel(parameters);
        model.prepare();
        try
        {
            parameters.max_array_size = 1;
            double[] result = model.predict(new[] {0.2, 0.2, 0.2, 0.2, 0.2});
            if (result.Length != 2 || result[0] < 0 || result[1] < 0 ||
                double.IsNaN(result[0]) || double.IsNaN(result[1]) ||
                Math.Abs(result[0] + result[1] - 1) > 1e-6)
                throw new Exception("Invalid probabilities after mutating model params");
        }
        finally { model.release(); }
        Console.WriteLine("ML capacity snapshot validation passed.");
    }

    static void Main(string[] args)
    {
        var data = new double[2, 256];
        Reject(() => DataFilter.perform_fft(data, 1, 0, 512, 0));
        Reject(() => DataFilter.get_psd(data, 1, 0, 512, 256, 0));
        Reject(() => DataFilter.calc_stddev(data, 0, -1, 10));
        Reject(() => DataFilter.calc_stddev(data, 1, 0, 512));
        Reject(() => DataFilter.calc_stddev(new double[4], 0, 8));
        Reject(() => DataFilter.get_band_power(Tuple.Create(new double[3], new double[2]), 0, 1));
        Reject(() => DataFilter.get_csp(new double[2, 2, 8], new double[1]));
        Reject(() => DataFilter.perform_wavelet_transform(new double[256], 3, -1, 0));
        Reject(() => DataFilter.perform_inverse_wavelet_transform(Tuple.Create(new double[4], new[] {2, 2}), 8, 3, 2, 0));
        foreach (int originalLength in new[] {4, int.MaxValue / 2, int.MaxValue})
            Reject(() => DataFilter.perform_inverse_wavelet_transform(Tuple.Create(new double[2], new[] {1, 1}), originalLength, 0, 1, 0));
        foreach (int capacity in new[] {0, -1})
        {
            var parameters = new BrainFlowModelParams(0, 0);
            parameters.max_array_size = capacity;
            Reject(() => new MLModel(parameters));
        }
        Reject(() => new MLModel(null));
        if (args.Length > 0)
            CheckMLSnapshot(args[0]);
        Console.WriteLine("Signal-processing binding validation passed.");
    }
}

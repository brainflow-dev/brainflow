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

    static void CheckChannelPacking()
    {
        var data = new double[4, 1024];
        for (int row = 0; row < 4; row++)
            for (int col = 0; col < 1024; col++)
                data[row, col] = Math.Sin(2 * Math.PI * (5 + row * 7) * col / 256.0);
        var original = (double[,])data.Clone();
        int[] channels = {3, 1, 3};
        var selected = new double[channels.Length, 1024];
        for (int row = 0; row < channels.Length; row++)
            for (int col = 0; col < 1024; col++)
                selected[row, col] = data[channels[row], col];
        var bands = new[] {Tuple.Create(2.0, 15.0), Tuple.Create(15.0, 40.0)};
        var actual = DataFilter.get_custom_band_powers(data, bands, channels, 256, false);
        var expected = DataFilter.get_custom_band_powers(selected, bands, new[] {0, 1, 2}, 256, false);
        for (int band = 0; band < bands.Length; band++)
            if (actual.Item1[band] != expected.Item1[band] || actual.Item2[band] != expected.Item2[band])
                throw new Exception("Selected/repeated channel powers changed");
        for (int row = 0; row < 4; row++)
            for (int col = 0; col < 1024; col++)
                if (data[row, col] != original[row, col])
                    throw new Exception("Channel packing modified input");
        Reject(() => DataFilter.get_custom_band_powers(data, bands, new[] {-1}, 256, false));
        Reject(() => DataFilter.get_custom_band_powers(data, bands, new[] {4}, 256, false));
        Reject(() => DataFilter.perform_ica(data, 2, new[] {0, 4}));
    }

    static void Main(string[] args)
    {
        CheckChannelPacking();
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

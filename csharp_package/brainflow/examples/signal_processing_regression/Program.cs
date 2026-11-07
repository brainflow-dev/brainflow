using System;
using System.Linq;
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

    static void CheckSignalQuality()
    {
        const int samples = 1024;
        const int rate = 128;
        var data = new double[3, samples];
        for (int i = 0; i < samples; i++)
        {
            data[0, i] = Math.Sin(2 * Math.PI * 10 * i / rate);
            data[2, i] = 2 * Math.Sin(2 * Math.PI * 20 * i / rate);
        }
        data[1, 0] = double.NaN;
        data[1, 1] = 100;
        data[1, 2] = -100;
        var options = new SignalQualityOptions(true, -10, 10, min_channels: 2);
        int[] channels = {2, 1, 0};
        var result = DataFilter.get_avg_band_powers_with_quality(data, channels, rate, false, options);
        var expected = DataFilter.get_avg_band_powers(data, new[] {2, 0}, rate, false);
        if (!result.valid || !result.used_channels.SequenceEqual(new[] {2, 0}) ||
            !result.rejected_channels.SequenceEqual(new[] {1}) ||
            !result.avg_band_powers.SequenceEqual(expected.Item1) ||
            !result.stddev_band_powers.SequenceEqual(expected.Item2))
            throw new Exception("Quality-aware powers differ from the accepted channel subset");
        var report = DataFilter.get_signal_quality(data, channels, rate, options).quality[1];
        if (report.channel != 1 || report.flags != (SignalQualityFlags.NONFINITE | SignalQualityFlags.CLIPPED | SignalQualityFlags.FLATLINE) ||
            report.nonfinite_percentage != 100.0 / samples || report.clipped_percentage != 200.0 / samples ||
            report.longest_flatline_seconds != (samples - 4.0) / rate)
            throw new Exception("Signal quality report was marshaled incorrectly");
        var invalid = DataFilter.get_avg_band_powers_with_quality(data, channels, rate, false,
            new SignalQualityOptions(true, -10, 10, min_channels: 3));
        if (invalid.valid || invalid.reason != "insufficient_channels" ||
            invalid.avg_band_powers != null || invalid.stddev_band_powers != null)
            throw new Exception("Rejected window returned usable powers");
        Reject(() => DataFilter.get_signal_quality(data, new[] {0, 0}, rate));
    }

    static void Main(string[] args)
    {
        CheckSignalQuality();
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

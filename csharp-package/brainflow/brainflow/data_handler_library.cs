using System.Runtime.InteropServices;

namespace brainflow
{
    public enum FilterTypes
    {
        BUTTERWORTH = 0,
        CHEBYSHEV_TYPE_1 = 1,
        BESSEL = 2
    };

    class DataHandlerLibrary64
    {
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_lowpass (double[] data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple);
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_highpass (double[] data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple);
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_bandpass (double[] data, int len, int sampling_rate, double center_freq, double band_width, int order, int filter_type, double ripple);
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_bandstop (double[] data, int len, int sampling_rate, double center_freq, double band_width, int order, int filter_type, double ripple);
    }

    class DataHandlerLibrary32
    {
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_lowpass (double[] data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_highpass (double[] data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_bandpass (double[] data, int len, int sampling_rate, double center_freq, double band_width, int order, int filter_type, double ripple);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_bandstop (double[] data, int len, int sampling_rate, double center_freq, double band_width, int order, int filter_type, double ripple);
    }

    class DataHandlerLibrary
    {
        public static int perform_lowpass (double[] data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple)
        {
            if (System.Environment.Is64BitProcess)
                return DataHandlerLibrary64.perform_lowpass (data, len, sampling_rate, cutoff, order, filter_type, ripple);
            else
                return DataHandlerLibrary32.perform_lowpass (data, len, sampling_rate, cutoff, order, filter_type, ripple);
        }

        public static int perform_highpass (double[] data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple)
        {
            if (System.Environment.Is64BitProcess)
                return DataHandlerLibrary64.perform_highpass (data, len, sampling_rate, cutoff, order, filter_type, ripple);
            else
                return DataHandlerLibrary32.perform_highpass (data, len, sampling_rate, cutoff, order, filter_type, ripple);
        }

        public static int perform_bandpass (double[] data, int len, int sampling_rate, double center_freq, double band_width, int order, int filter_type, double ripple)
        {
            if (System.Environment.Is64BitProcess)
                return DataHandlerLibrary64.perform_bandpass (data, len, sampling_rate, center_freq, band_width, order, filter_type, ripple);
            else
                return DataHandlerLibrary32.perform_bandpass (data, len, sampling_rate, center_freq, band_width, order, filter_type, ripple);
        }

        public static int perform_bandstop (double[] data, int len, int sampling_rate, double center_freq, double band_width, int order, int filter_type, double ripple)
        {
            if (System.Environment.Is64BitProcess)
                return DataHandlerLibrary64.perform_bandstop (data, len, sampling_rate, center_freq, band_width, order, filter_type, ripple);
            else
                return DataHandlerLibrary32.perform_bandstop (data, len, sampling_rate, center_freq, band_width, order, filter_type, ripple);
        }

    }
}

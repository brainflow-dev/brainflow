using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace brainflow
{
    public enum BrainFlowMetrics
    {
        RELAXATION = 0,
        CONCENTRATION = 1
    };

    public enum BrainFlowClassifiers
    {
        REGRESSION = 0,
    };

    public static class MLModuleLibrary64
    {
        [DllImport("MLModule.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int prepare(string input_json);
        [DllImport("MLModule.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int release(string input_json);
        [DllImport("MLModule.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int predict(double[] data, int data_len, double[] output, string input_json);
    }

    public static class MLModuleLibrary32
    {
        [DllImport("MLModule32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int prepare(string input_json);
        [DllImport("MLModule32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int release(string input_json);
        [DllImport("MLModule32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int predict(double[] data, int data_len, double[] output, string input_json);
    }

    public static class MLModuleLibrary
    {
        public static int prepare (string input_json)
        {
            if (System.Environment.Is64BitProcess)
                return MLModuleLibrary64.prepare (input_json);
            else
                return MLModuleLibrary32.prepare (input_json);
        }

        public static int release (string input_json)
        {
            if (System.Environment.Is64BitProcess)
                return MLModuleLibrary64.release (input_json);
            else
                return MLModuleLibrary32.release (input_json);
        }

        public static int predict (double[] data, int data_len, double[] output, string input_json)
        {
            if (System.Environment.Is64BitProcess)
                return MLModuleLibrary64.predict (data, data_len,output, input_json);
            else
                return MLModuleLibrary32.predict (data, data_len, output, input_json);
        }
    }
}

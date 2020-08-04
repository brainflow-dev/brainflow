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
        ALGORITHMIC = 0,
        REGRESSION = 1,
        SVM = 2
    };

    public static class MLModuleLibrary64
    {
        [DllImport("MLModule.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int prepare(int metric, int classifier);
        [DllImport("MLModule.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int release(int metric, int classifier);
        [DllImport("MLModule.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int predict(double[] data, int data_len, double[] output, int metric, int classifier);
    }

    public static class MLModuleLibrary32
    {
        [DllImport("MLModule32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int prepare(int metric, int classifier);
        [DllImport("MLModule32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int release(int metric, int classifier);
        [DllImport("MLModule32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int predict(double[] data, int data_len, double[] output, int metric, int classifier);
    }

    public static class MLModuleLibrary
    {
        public static int prepare (int metric, int classifier)
        {
            if (System.Environment.Is64BitProcess)
                return MLModuleLibrary64.prepare (metric, classifier);
            else
                return MLModuleLibrary32.prepare (metric, classifier);
        }

        public static int release (int metric, int classifier)
        {
            if (System.Environment.Is64BitProcess)
                return MLModuleLibrary64.release (metric, classifier);
            else
                return MLModuleLibrary32.release (metric, classifier);
        }

        public static int predict (double[] data, int data_len, double[] output, int metric, int classifier)
        {
            if (System.Environment.Is64BitProcess)
                return MLModuleLibrary64.predict (data, data_len,output, metric, classifier);
            else
                return MLModuleLibrary32.predict (data, data_len, output, metric, classifier);
        }
    }
}

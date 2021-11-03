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
        CONCENTRATION = 1,
        USER_DEFINED = 2
    };

    public enum BrainFlowClassifiers
    {
        REGRESSION = 0,
        KNN = 1,
        SVM = 2,
        LDA = 3,
        DYN_LIB_CLASSIFIER = 4
    };

    public static class MLModuleLibrary64
    {
        [DllImport ("MLModule.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_file_ml_module (string log_file);
        [DllImport ("MLModule.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_level_ml_module (int log_level);
        [DllImport ("MLModule.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int prepare(string input_json);
        [DllImport ("MLModule.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int release(string input_json);
        [DllImport ("MLModule.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int predict(double[] data, int data_len, double[] output, string input_json);
    }

    public static class MLModuleLibrary32
    {
        [DllImport ("MLModule32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_file_ml_module (string log_file);
        [DllImport ("MLModule32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_level_ml_module (int log_level);
        [DllImport ("MLModule32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int prepare(string input_json);
        [DllImport ("MLModule32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int release(string input_json);
        [DllImport ("MLModule32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int predict(double[] data, int data_len, double[] output, string input_json);
    }

    public static class MLModuleLibraryLinux
    {
        [DllImport ("libMLModule.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_file_ml_module (string log_file);
        [DllImport ("libMLModule.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_level_ml_module (int log_level);
        [DllImport ("libMLModule.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int prepare (string input_json);
        [DllImport ("libMLModule.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int release (string input_json);
        [DllImport ("libMLModule.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int predict (double[] data, int data_len, double[] output, string input_json);
    }

    public static class MLModuleLibraryMac
    {
        [DllImport ("libMLModule.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_file_ml_module (string log_file);
        [DllImport ("libMLModule.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_level_ml_module (int log_level);
        [DllImport ("libMLModule.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int prepare (string input_json);
        [DllImport ("libMLModule.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int release (string input_json);
        [DllImport ("libMLModule.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int predict (double[] data, int data_len, double[] output, string input_json);
    }


    public static class MLModuleLibrary
    {
        public static int set_log_level_ml_module (int log_level)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return MLModuleLibrary64.set_log_level_ml_module (log_level);
                case LibraryEnvironment.x86:
                    return MLModuleLibrary32.set_log_level_ml_module (log_level);
                case LibraryEnvironment.Linux:
                    return MLModuleLibraryLinux.set_log_level_ml_module (log_level);
                case LibraryEnvironment.MacOS:
                    return MLModuleLibraryMac.set_log_level_ml_module (log_level);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int set_log_file_ml_module (string log_file)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return MLModuleLibrary64.set_log_file_ml_module (log_file);
                case LibraryEnvironment.x86:
                    return MLModuleLibrary32.set_log_file_ml_module (log_file);
                case LibraryEnvironment.Linux:
                    return MLModuleLibraryLinux.set_log_file_ml_module (log_file);
                case LibraryEnvironment.MacOS:
                    return MLModuleLibraryMac.set_log_file_ml_module (log_file);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int prepare (string input_json)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return MLModuleLibrary64.prepare (input_json);
                case LibraryEnvironment.x86:
                    return MLModuleLibrary32.prepare (input_json);
                case LibraryEnvironment.Linux:
                    return MLModuleLibraryLinux.prepare (input_json);
                case LibraryEnvironment.MacOS:
                    return MLModuleLibraryMac.prepare (input_json);

            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int release (string input_json)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return MLModuleLibrary64.release (input_json);
                case LibraryEnvironment.x86:
                    return MLModuleLibrary32.release (input_json);
                case LibraryEnvironment.Linux:
                    return MLModuleLibraryLinux.release (input_json);
                case LibraryEnvironment.MacOS:
                    return MLModuleLibraryMac.release (input_json);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int predict (double[] data, int data_len, double[] output, string input_json)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return MLModuleLibrary64.predict (data, data_len, output, input_json);
                case LibraryEnvironment.x86:
                    return MLModuleLibrary32.predict (data, data_len, output, input_json);
                case LibraryEnvironment.Linux:
                    return MLModuleLibraryLinux.predict (data, data_len, output, input_json);
                case LibraryEnvironment.MacOS:
                    return MLModuleLibraryMac.predict (data, data_len, output, input_json);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }
    }
}

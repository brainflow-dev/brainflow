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
        MINDFULNESS = 0,
        RESTFULNESS = 1,
        USER_DEFINED = 2
    };

    public enum BrainFlowClassifiers
    {
        DEFAULT_CLASSIFIER = 0,
        DYN_LIB_CLASSIFIER = 1,
        ONNX_CLASSIFIER = 2
    };

    public static class MLModuleLibrary64
    {
        [DllImport ("MLModule", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_file_ml_module (string log_file);
        [DllImport ("MLModule", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int log_message_ml_module (int log_level, string message);
        [DllImport ("MLModule", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_level_ml_module (int log_level);
        [DllImport ("MLModule", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int prepare (string input_json);
        [DllImport ("MLModule", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int release (string input_json);
        [DllImport ("MLModule", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int predict (double[] data, int data_len, double[] output, int[] output_len, string input_json);
        [DllImport ("MLModule", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int release_all ();
        [DllImport ("MLModule", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_version_ml_module (byte[] version, int[] len, int max_len);
    }

    public static class MLModuleLibrary32
    {
        [DllImport ("MLModule32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_file_ml_module (string log_file);
        [DllImport ("MLModule32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int log_message_ml_module (int log_level, string message);
        [DllImport ("MLModule32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_level_ml_module (int log_level);
        [DllImport ("MLModule32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int prepare (string input_json);
        [DllImport ("MLModule32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int release (string input_json);
        [DllImport ("MLModule32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int predict (double[] data, int data_len, double[] output, int[] output_len, string input_json);
        [DllImport ("MLModule32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int release_all ();
        [DllImport ("MLModule32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_version_ml_module (byte[] version, int[] len, int max_len);
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
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int set_log_file_ml_module (string log_file)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return MLModuleLibrary64.set_log_file_ml_module (log_file);
                case LibraryEnvironment.x86:
                    return MLModuleLibrary32.set_log_file_ml_module (log_file);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int prepare (string input_json)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return MLModuleLibrary64.prepare (input_json);
                case LibraryEnvironment.x86:
                    return MLModuleLibrary32.prepare (input_json);

            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int release (string input_json)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return MLModuleLibrary64.release (input_json);
                case LibraryEnvironment.x86:
                    return MLModuleLibrary32.release (input_json);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int predict (double[] data, int data_len, double[] output, int[] output_len, string input_json)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return MLModuleLibrary64.predict (data, data_len, output, output_len, input_json);
                case LibraryEnvironment.x86:
                    return MLModuleLibrary32.predict (data, data_len, output, output_len, input_json);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int release_all ()
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return MLModuleLibrary64.release_all ();
                case LibraryEnvironment.x86:
                    return MLModuleLibrary32.release_all ();
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_version_ml_module (byte[] version, int[] len, int max_len)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return MLModuleLibrary64.get_version_ml_module (version, len, max_len);
                case LibraryEnvironment.x86:
                    return MLModuleLibrary32.get_version_ml_module (version, len, max_len);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int log_message_ml_module (int log_level, string message)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return MLModuleLibrary64.log_message_ml_module (log_level, message);
                case LibraryEnvironment.x86:
                    return MLModuleLibrary32.log_message_ml_module (log_level, message);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }
    }
}

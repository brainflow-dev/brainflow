using System.Runtime.InteropServices;

namespace brainflow
{
    public enum CustomExitCodes
    {
        STATUS_OK = 0,
        PORT_ALREADY_OPEN_ERROR,
        UNABLE_TO_OPEN_PORT_ERROR,
        SET_PORT_ERROR,
        BOARD_WRITE_ERROR,
        INCOMMING_MSG_ERROR,
        INITIAL_MSG_ERROR,
        BOARD_NOT_READY_ERROR,
        STREAM_ALREADY_RUN_ERROR,
        INVALID_BUFFER_SIZE_ERROR,
        STREAM_THREAD_ERROR,
        STREAM_THREAD_IS_NOT_RUNNING,
        EMPTY_BUFFER_ERROR,
        INVALID_ARGUMENTS_ERROR,
        UNSUPPORTED_BOARD_ERROR,
        BOARD_NOT_CREATED_ERROR,
        ANOTHER_BOARD_IS_CREATED_ERROR,
        GENERAL_ERROR,
        SYNC_TIMEOUT_ERROR
    };

    public enum BoardIds
    {
        SYNTHETIC_BOARD = -1,
        CYTON_BOARD = 0,
        GANGLION_BOARD = 1,
        CYTON_DAISY_BOARD = 2,
        NOVAXR_BOARD = 3,
        GANGLION_WIFI = 4,
        CYTON_WIFI = 5,
        CYTON_DAISY_WIFI = 6
    };


    // lib name is const in DllImport directive, so I can not use only one class
    // alternative solutios: call loadlibrary from kernel32.dll
    // or rename libs to have the same name and load them from different folders but I dont want to fix it in all bindings
    // so there is no really good solution
    public static class Library64
    {
        [DllImport ("BoardController.dll", SetLastError = true)]
        public static extern int prepare_session (int board_id, string port_name);
        [DllImport ("BoardController.dll", SetLastError = true)]
        public static extern int start_stream (int buffer_size, int board_id, string port_name);
        [DllImport ("BoardController.dll", SetLastError = true)]
        public static extern int stop_stream (int board_id, string port_name);
        [DllImport ("BoardController.dll", SetLastError = true)]
        public static extern int release_session (int board_id, string port_name);
        [DllImport ("BoardController.dll", SetLastError = true)]
        public static extern int get_current_board_data (int num_samples, float[] data_buf, double[] ts_buf, int[] returned_samples, int board_id, string port_name);
        [DllImport ("BoardController.dll", SetLastError = true)]
        public static extern int get_board_data_count(int[] result, int board_id, string port_name);
        [DllImport ("BoardController.dll", SetLastError = true)]
        public static extern int get_board_data (int data_count, float[] data_buf, double[] ts_buf, int board_id, string port_name);
        [DllImport ("BoardController.dll", SetLastError = true)]
        public static extern int set_log_level (int log_level);
        [DllImport ("BoardController.dll", SetLastError = true)]
        public static extern int config_board (string config, int board_id, string port_name);
        [DllImport ("BoardController.dll", SetLastError = true)]
        public static extern int set_log_file (string log_file);
    }

    public static class Library32
    {
        [DllImport ("BoardController32.dll", SetLastError = true)]
        public static extern int prepare_session (int board_id, string port_name);
        [DllImport ("BoardController32.dll", SetLastError = true)]
        public static extern int start_stream (int buffer_size, int board_id, string port_name);
        [DllImport ("BoardController32.dll", SetLastError = true)]
        public static extern int stop_stream (int board_id, string port_name);
        [DllImport ("BoardController32.dll", SetLastError = true)]
        public static extern int release_session (int board_id, string port_name);
        [DllImport ("BoardController32.dll", SetLastError = true)]
        public static extern int get_current_board_data (int num_samples, float[] data_buf, double[] ts_buf, int[] returned_samples, int board_id, string port_name);
        [DllImport ("BoardController32.dll", SetLastError = true)]
        public static extern int get_board_data_count (int[] result, int board_id, string port_name);
        [DllImport ("BoardController32.dll", SetLastError = true)]
        public static extern int get_board_data (int data_count, float[] data_buf, double[] ts_buf, int board_id, string port_name);
        [DllImport ("BoardController32.dll", SetLastError = true)]
        public static extern int set_log_level (int log_level);
        [DllImport ("BoardController32.dll", SetLastError = true)]
        public static extern int config_board (string config, int board_id, string port_name);
        [DllImport ("BoardController32.dll", SetLastError = true)]
        public static extern int set_log_file (string log_file);
    }

    public static class Library
    {
        public static int prepare_session (int board_id, string port_name)
        {
            if (System.Environment.Is64BitProcess)
                return Library64.prepare_session (board_id, port_name);
            else
                return Library32.prepare_session (board_id, port_name);
        }

        public static int start_stream (int buffer_size, int board_id, string port_name)
        {
            if (System.Environment.Is64BitProcess)
                return Library64.start_stream (buffer_size, board_id, port_name);
            else
                return Library32.start_stream (buffer_size, board_id, port_name);
        }

        public static int stop_stream (int board_id, string port_name)
        {
            if (System.Environment.Is64BitProcess)
                return Library64.stop_stream (board_id, port_name);
            else
                return Library32.stop_stream (board_id, port_name);
        }

        public static int release_session (int board_id, string port_name)
        {
            if (System.Environment.Is64BitProcess)
                return Library64.release_session (board_id, port_name);
            else
                return Library32.release_session (board_id, port_name);
        }

        public static int get_current_board_data (int num_samples, float[] data_buf, double[] ts_buf, int[] returned_samples, int board_id, string port_name)
        {
            if (System.Environment.Is64BitProcess)
                return Library64.get_current_board_data (num_samples, data_buf, ts_buf, returned_samples, board_id, port_name);
            else
                return Library32.get_current_board_data (num_samples, data_buf, ts_buf, returned_samples, board_id, port_name);
        }

        public static int get_board_data_count (int[] result, int board_id, string port_name)
        {
            if (System.Environment.Is64BitProcess)
                return Library64.get_board_data_count (result, board_id, port_name);
            else
                return Library32.get_board_data_count (result, board_id, port_name);
        }

        public static int get_board_data (int data_count, float[] data_buf, double[] ts_buf, int board_id, string port_name)
        {
            if (System.Environment.Is64BitProcess)
                return Library64.get_board_data (data_count, data_buf, ts_buf, board_id, port_name);
            else
                return Library32.get_board_data (data_count, data_buf, ts_buf, board_id, port_name);
        }

        public static int set_log_level (int log_level)
        {
            if (System.Environment.Is64BitProcess)
                return Library64.set_log_level (log_level);
            else
                return Library32.set_log_level (log_level);
        }

        public static int config_board (string config, int board_id, string port_name)
        {
            if (System.Environment.Is64BitProcess)
                return Library64.config_board (config, board_id, port_name);
            else
                return Library32.config_board (config, board_id, port_name);
        }

        public static int set_log_file (string log_file)
        {
            if (System.Environment.Is64BitProcess)
                return Library64.set_log_file (log_file);
            else
                return Library32.set_log_file (log_file);
        }
    }
}

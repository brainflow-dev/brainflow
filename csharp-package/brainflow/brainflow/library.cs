using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

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
        CYTON_BOARD = 0,
        GANGLION_BOARD = 1
    };

    public static class Library
    {
        [DllImport("BoardController.dll", SetLastError = true)]
        public static extern int prepare_session(int board_id, string port_name);
        [DllImport("BoardController.dll", SetLastError = true)]
        public static extern int start_stream(int buffer_size);
        [DllImport("BoardController.dll", SetLastError = true)]
        public static extern int stop_stream();
        [DllImport("BoardController.dll", SetLastError = true)]
        public static extern int release_session();
        [DllImport("BoardController.dll", SetLastError = true)]
        public static extern int get_current_board_data(int num_samples, float[] data_buf, double[] ts_buf, int[] returned_samples);
        [DllImport("BoardController.dll", SetLastError = true)]
        public static extern int get_board_data_count(int[] result);
        [DllImport("BoardController.dll", SetLastError = true)]
        public static extern int get_board_data(int data_count, float[] data_buf, double[] ts_buf);
    }

}

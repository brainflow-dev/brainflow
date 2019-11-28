using System;
using brainflow;

using Accord.Math;

namespace test
{
    class GetBoardData
    {
        static void Main (string[] args)
        {
            // use synthetic board for demo
            BoardShim.enable_dev_board_logger ();
            BrainFlowInputParams input_params = new BrainFlowInputParams ();
            int board_id = (int)BoardIds.SYNTHETIC_BOARD;

            BoardShim board_shim = new BoardShim (board_id, input_params);
            board_shim.prepare_session ();
            board_shim.start_stream (3600);
            BoardShim.log_message ((int)LogLevels.LEVEL_INFO, "Start sleeping in the main thread");
            System.Threading.Thread.Sleep (5000);
            board_shim.stop_stream ();
            Console.WriteLine ("data count: {0}", board_shim.get_board_data_count ());
            double[,] unprocessed_data = board_shim.get_current_board_data (20);
            int[] eeg_channels = BoardShim.get_eeg_channels (board_id);
            Console.WriteLine ("Before serialization:");
            foreach (var index in eeg_channels)
                Console.WriteLine ("[{0}]", string.Join (", ", unprocessed_data.GetRow (index)));
            board_shim.release_session ();

            // demo for data serialization
            DataFilter.write_file (unprocessed_data, "test.csv", "w");
            double[,] restored_data = DataFilter.read_file ("test.csv");
            Console.WriteLine ("After Serialization:");
            foreach (var index in eeg_channels)
                Console.WriteLine ("[{0}]", string.Join (", ", restored_data.GetRow (index)));
        }
    }
}

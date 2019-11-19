using System;
using brainflow;

using Accord.Math;

namespace test
{
    class GetBoardData
    {
        static void Main (string[] args)
        {
            BoardShim.enable_dev_board_logger ();

            BrainFlowInputParams input_params = new BrainFlowInputParams ();
            int board_id = parse_args (args, input_params);

            BoardShim board_shim = new BoardShim (board_id, input_params);
            board_shim.prepare_session ();
            board_shim.start_stream (3600);
            BoardShim.log_message ((int)LogLevels.LEVEL_INFO, "Start sleeping in the main thread");
            System.Threading.Thread.Sleep (5000);
            board_shim.stop_stream ();
            Console.WriteLine ("data count: {0}", board_shim.get_board_data_count ());
            double[,] unprocessed_data = board_shim.get_current_board_data (20);
            int[] eeg_channels = BoardShim.get_eeg_channels (board_id);
            Console.WriteLine ("Before processing:");
            foreach (var index in eeg_channels)
                Console.WriteLine ("[{0}]", string.Join (", ", unprocessed_data.GetRow (index)));
            board_shim.release_session ();

            // demo for data serialization
            DataFilter.write_file (unprocessed_data, "test.csv", "w");
            double[,] restored_data = DataFilter.read_file ("test.csv");
            Console.WriteLine ("After Serialization:");
            foreach (var index in eeg_channels)
                Console.WriteLine ("[{0}]", string.Join (", ", restored_data.GetRow (index)));

            // for demo apply different filters to different channels
            double[] filtered;
            for (int i = 0; i < eeg_channels.Length; i++)
            {
                switch (i)
                {
                    case 0:
                        filtered = DataFilter.perform_lowpass (unprocessed_data.GetRow(eeg_channels[i]), BoardShim.get_sampling_rate (board_id), 20.0, 4, (int)FilterTypes.BESSEL, 0.0);
                        Console.WriteLine ("Filtered channel " + eeg_channels[i]);
                        Console.WriteLine ("[{0}]", string.Join (", ", filtered));
                        break;
                    case 1:
                        filtered = DataFilter.perform_highpass (unprocessed_data.GetRow (eeg_channels[i]), BoardShim.get_sampling_rate (board_id), 2.0, 4, (int)FilterTypes.BUTTERWORTH, 0.0);
                        Console.WriteLine ("Filtered channel " + eeg_channels[i]);
                        Console.WriteLine ("[{0}]", string.Join (", ", filtered));
                        break;
                    case 2:
                        filtered = DataFilter.perform_bandpass (unprocessed_data.GetRow (eeg_channels[i]), BoardShim.get_sampling_rate (board_id), 20.0, 5.0, 2, (int)FilterTypes.BUTTERWORTH, 0.0);
                        Console.WriteLine ("Filtered channel " + eeg_channels[i]);
                        Console.WriteLine ("[{0}]", string.Join (", ", filtered));
                        break;
                    case 3:
                        filtered = DataFilter.perform_bandstop (unprocessed_data.GetRow (eeg_channels[i]), BoardShim.get_sampling_rate (board_id), 20.0, 1.0, 6, (int)FilterTypes.CHEBYSHEV_TYPE_1, 1.0);
                        Console.WriteLine ("Filtered channel " + eeg_channels[i]);
                        Console.WriteLine ("[{0}]", string.Join (", ", filtered));
                        break;
                    case 4:
                        filtered = DataFilter.perform_rolling_filter (unprocessed_data.GetRow (eeg_channels[i]), 3, (int)AggOperations.MEAN);
                        Console.WriteLine ("Filtered channel " + eeg_channels[i]);
                        Console.WriteLine ("[{0}]", string.Join (", ", filtered));
                        break;
                    case 5:
                        filtered = DataFilter.perform_rolling_filter (unprocessed_data.GetRow (eeg_channels[i]), 3, (int)AggOperations.MEDIAN);
                        Console.WriteLine ("Filtered channel " + eeg_channels[i]);
                        Console.WriteLine ("[{0}]", string.Join (", ", filtered));
                        break;
                    case 6:
                        filtered = DataFilter.perform_downsampling (unprocessed_data.GetRow (eeg_channels[i]), 3, (int)AggOperations.MEDIAN);
                        Console.WriteLine ("Filtered channel " + eeg_channels[i]);
                        Console.WriteLine ("[{0}]", string.Join (", ", filtered));
                        break;
                }
            }
        }

        static int parse_args (string[] args, BrainFlowInputParams input_params)
        {
            int board_id = (int)BoardIds.SYNTHETIC_BOARD; //assume synthetic board by default
            // use docs to get params for your specific board, e.g. set serial_port for Cyton
            for (int i = 0; i < args.Length; i++)
            {
                if (args[i].Equals ("--ip-address"))
                {
                    input_params.ip_address = args[i + 1];
                }
                if (args[i].Equals ("--mac-address"))
                {
                    input_params.mac_address = args[i + 1];
                }
                if (args[i].Equals ("--serial-port"))
                {
                    input_params.serial_port = args[i + 1];
                }
                if (args[i].Equals ("--other-info"))
                {
                    input_params.other_info = args[i + 1];
                }
                if (args[i].Equals ("--ip-port"))
                {
                    input_params.ip_port = Convert.ToInt32 (args[i + 1]);
                }
                if (args[i].Equals ("--ip-protocol"))
                {
                    input_params.ip_protocol = Convert.ToInt32 (args[i + 1]);
                }
                if (args[i].Equals ("--board-id"))
                {
                    board_id = Convert.ToInt32 (args[i + 1]);
                }
            }
            return board_id;
        }
    }
}

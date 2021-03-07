using System;

using brainflow;
using brainflow.math;


namespace test
{
    class EEGMetricsCi
    {
        static void Main (string[] args)
        {
            // use synthetic board for demo
            BoardShim.enable_dev_board_logger ();
            BrainFlowInputParams input_params = new BrainFlowInputParams ();
            BrainFlowModelParams model_params = new BrainFlowModelParams (0,0);
            int board_id = parse_args (args, input_params,model_params);
            BoardShim board_shim = new BoardShim (board_id, input_params);
            int sampling_rate = BoardShim.get_sampling_rate (board_shim.get_board_id ());
            int nfft = DataFilter.get_nearest_power_of_two (sampling_rate);
            int[] eeg_channels = BoardShim.get_eeg_channels (board_shim.get_board_id ());

            board_shim.prepare_session ();
            board_shim.start_stream (3600);
            System.Threading.Thread.Sleep (10000);
            board_shim.stop_stream ();
            double[,] data = board_shim.get_board_data ();
            board_shim.release_session ();

            Tuple<double[], double[]> bands = DataFilter.get_avg_band_powers (data, eeg_channels, sampling_rate, true);
            double[] feature_vector = bands.Item1.Concatenate (bands.Item2);
            MLModel concentration = new MLModel (model_params);
            concentration.prepare ();
            Console.WriteLine (Enum.GetName (typeof (BrainFlowMetrics), model_params.metric) + " " + Enum.GetName (typeof (BrainFlowClassifiers), model_params.classifier) +
                " : "  + concentration.predict (feature_vector));
            concentration.release ();
        }

        static int parse_args (string[] args, BrainFlowInputParams input_params,  BrainFlowModelParams model_params)
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
                if (args[i].Equals ("--timeout"))
                {
                    input_params.timeout = Convert.ToInt32 (args[i + 1]);
                }
                if (args[i].Equals ("--serial-number"))
                {
                    input_params.serial_number = args[i + 1];
                }
                if (args[i].Equals ("--file"))
                {
                    input_params.file = args[i + 1];
                }
                if (args[i].Equals ("--metric"))
                {
                    model_params.metric = Convert.ToInt32 (args[i + 1]);
                }
                if (args[i].Equals ("--classifier"))
                {
                    model_params.classifier = Convert.ToInt32 (args[i + 1]);
                }
            }
            return board_id;
        }
    }
}

package brainflow.examples;

import org.apache.commons.lang3.tuple.Pair;

import brainflow.BoardShim;
import brainflow.BrainFlowClassifiers;
import brainflow.BrainFlowInputParams;
import brainflow.BrainFlowMetrics;
import brainflow.BrainFlowModelParams;
import brainflow.DataFilter;
import brainflow.LogLevels;
import brainflow.MLModel;

public class EEGMetrics
{

    public static void main (String[] args) throws Exception
    {
        BoardShim.enable_board_logger ();
        BrainFlowInputParams params = new BrainFlowInputParams ();
        int board_id = parse_args (args, params);
        BoardShim board_shim = new BoardShim (board_id, params);
        int master_board_id = board_shim.get_board_id ();
        int sampling_rate = BoardShim.get_sampling_rate (master_board_id);
        int[] eeg_channels = BoardShim.get_eeg_channels (master_board_id);

        board_shim.prepare_session ();
        board_shim.start_stream (3600);
        BoardShim.log_message (LogLevels.LEVEL_INFO, "Start sleeping in the main thread");
        // recommended window size for eeg metric calculation is at least 4 seconds,
        // bigger is better
        Thread.sleep (5000);
        board_shim.stop_stream ();
        double[][] data = board_shim.get_board_data ();
        board_shim.release_session ();

        Pair<double[], double[]> bands = DataFilter.get_avg_band_powers (data, eeg_channels, sampling_rate, true);
        double[] feature_vector = bands.getLeft ();
        BrainFlowModelParams model_params = new BrainFlowModelParams (BrainFlowMetrics.MINDFULNESS,
                BrainFlowClassifiers.DEFAULT_CLASSIFIER);
        MLModel model = new MLModel (model_params);
        model.prepare ();
        System.out.print ("Score: " + model.predict (feature_vector)[0]);
        model.release ();
    }

    private static int parse_args (String[] args, BrainFlowInputParams params)
    {
        int board_id = -1;
        for (int i = 0; i < args.length; i++)
        {
            String arg = args[i];
            if (arg.equals ("--ip-address") || arg.equals ("--serial-port") || arg.equals ("--ip-port")
                    || arg.equals ("--ip-protocol") || arg.equals ("--other-info") || arg.equals ("--board-id")
                    || arg.equals ("--timeout") || arg.equals ("--serial-number") || arg.equals ("--file"))
            {
                if (i + 1 >= args.length)
                {
                    throw new IllegalArgumentException ("Missing value for argument: " + arg);
                }
                String value = args[++i];
                if (arg.equals ("--ip-address"))
                {
                    params.ip_address = value;
                }
                else if (arg.equals ("--serial-port"))
                {
                    params.serial_port = value;
                }
                else if (arg.equals ("--ip-port"))
                {
                    params.ip_port = Integer.parseInt (value);
                }
                else if (arg.equals ("--ip-protocol"))
                {
                    params.ip_protocol = Integer.parseInt (value);
                }
                else if (arg.equals ("--other-info"))
                {
                    params.other_info = value;
                }
                else if (arg.equals ("--board-id"))
                {
                    board_id = Integer.parseInt (value);
                }
                else if (arg.equals ("--timeout"))
                {
                    params.timeout = Integer.parseInt (value);
                }
                else if (arg.equals ("--serial-number"))
                {
                    params.serial_number = value;
                }
                else if (arg.equals ("--file"))
                {
                    params.file = value;
                }
            }
        }
        return board_id;
    }
}

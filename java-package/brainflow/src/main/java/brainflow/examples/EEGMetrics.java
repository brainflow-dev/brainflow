package brainflow.examples;

import org.apache.commons.lang3.ArrayUtils;
import org.apache.commons.lang3.tuple.Pair;

import brainflow.BoardIds;
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
        BoardShim.log_message (LogLevels.LEVEL_INFO.get_code (), "Start sleeping in the main thread");
        // recommended window size for eeg metric calculation is at least 4 seconds,
        // bigger is better
        Thread.sleep (5000);
        board_shim.stop_stream ();
        double[][] data = board_shim.get_board_data ();
        board_shim.release_session ();

        Pair<double[], double[]> bands = DataFilter.get_avg_band_powers (data, eeg_channels, sampling_rate, true);
        double[] feature_vector = ArrayUtils.addAll (bands.getLeft (), bands.getRight ());
        BrainFlowModelParams model_params = new BrainFlowModelParams (BrainFlowMetrics.CONCENTRATION.get_code (),
                BrainFlowClassifiers.REGRESSION.get_code ());
        MLModel concentration = new MLModel (model_params);
        concentration.prepare ();
        System.out.print ("Concentration: " + concentration.predict (feature_vector));
        concentration.release ();
    }

    private static int parse_args (String[] args, BrainFlowInputParams params)
    {
        int board_id = -1;
        for (int i = 0; i < args.length; i++)
        {
            if (args[i].equals ("--ip-address"))
            {
                params.ip_address = args[i + 1];
            }
            if (args[i].equals ("--serial-port"))
            {
                params.serial_port = args[i + 1];
            }
            if (args[i].equals ("--ip-port"))
            {
                params.ip_port = Integer.parseInt (args[i + 1]);
            }
            if (args[i].equals ("--ip-protocol"))
            {
                params.ip_protocol = Integer.parseInt (args[i + 1]);
            }
            if (args[i].equals ("--other-info"))
            {
                params.other_info = args[i + 1];
            }
            if (args[i].equals ("--board-id"))
            {
                board_id = Integer.parseInt (args[i + 1]);
            }
            if (args[i].equals ("--timeout"))
            {
                params.timeout = Integer.parseInt (args[i + 1]);
            }
            if (args[i].equals ("--serial-number"))
            {
                params.serial_number = args[i + 1];
            }
            if (args[i].equals ("--file"))
            {
                params.file = args[i + 1];
            }
        }
        return board_id;
    }
}

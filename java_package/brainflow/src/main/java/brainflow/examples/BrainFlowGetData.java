package brainflow.examples;

import java.util.Arrays;

import brainflow.BoardShim;
import brainflow.BrainFlowInputParams;
import brainflow.LogLevels;

public class BrainFlowGetData
{

    public static void main (String[] args) throws Exception
    {
        BoardShim.enable_board_logger ();
        BrainFlowInputParams params = new BrainFlowInputParams ();
        int board_id = parse_args (args, params);
        BoardShim board_shim = new BoardShim (board_id, params);

        board_shim.prepare_session ();
        // board_shim.start_stream (); // use this for default options
        board_shim.start_stream (450000, "file://file_stream.csv:w");
        BoardShim.log_message (LogLevels.LEVEL_INFO.get_code (), "Start sleeping in the main thread");
        Thread.sleep (5000);
        board_shim.stop_stream ();
        System.out.println (board_shim.get_board_data_count ());
        double[][] data = board_shim.get_current_board_data (30); // doesnt flush it from ring buffer
        // double[][] data = board_shim.get_board_data (); // get all data and flush
        // from ring buffer
        for (int i = 0; i < data.length; i++)
        {
            System.out.println (Arrays.toString (data[i]));
        }
        board_shim.release_session ();
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

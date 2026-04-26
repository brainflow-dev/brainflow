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
            String arg = args[i];
            if ((arg.equals ("--ip-address") || arg.equals ("--ip-address-aux") || arg.equals ("--ip-address-anc")
                    || arg.equals ("--serial-port") || arg.equals ("--ip-port") || arg.equals ("--ip-port-aux")
                    || arg.equals ("--ip-port-anc") || arg.equals ("--ip-protocol") || arg.equals ("--other-info")
                    || arg.equals ("--board-id") || arg.equals ("--timeout") || arg.equals ("--serial-number")
                    || arg.equals ("--file") || arg.equals ("--file-aux") || arg.equals ("--file-anc")
                    || arg.equals ("--master-board")) && (i + 1 >= args.length))
            {
                throw new IllegalArgumentException ("Missing value for argument: " + arg);
            }

            if (arg.equals ("--ip-address"))
            {
                params.ip_address = args[i + 1];
            }
            if (arg.equals ("--ip-address-aux"))
            {
                params.ip_address_aux = args[i + 1];
            }
            if (arg.equals ("--ip-address-anc"))
            {
                params.ip_address_anc = args[i + 1];
            }
            if (arg.equals ("--serial-port"))
            {
                params.serial_port = args[i + 1];
            }
            if (arg.equals ("--ip-port"))
            {
                params.ip_port = Integer.parseInt (args[i + 1]);
            }
            if (arg.equals ("--ip-port-aux"))
            {
                params.ip_port_aux = Integer.parseInt (args[i + 1]);
            }
            if (arg.equals ("--ip-port-anc"))
            {
                params.ip_port_anc = Integer.parseInt (args[i + 1]);
            }
            if (arg.equals ("--ip-protocol"))
            {
                params.ip_protocol = Integer.parseInt (args[i + 1]);
            }
            if (arg.equals ("--other-info"))
            {
                params.other_info = args[i + 1];
            }
            if (arg.equals ("--board-id"))
            {
                board_id = Integer.parseInt (args[i + 1]);
            }
            if (arg.equals ("--timeout"))
            {
                params.timeout = Integer.parseInt (args[i + 1]);
            }
            if (arg.equals ("--serial-number"))
            {
                params.serial_number = args[i + 1];
            }
            if (arg.equals ("--file"))
            {
                params.file = args[i + 1];
            }
            if (arg.equals ("--file-aux"))
            {
                params.file_aux = args[i + 1];
            }
            if (arg.equals ("--file-anc"))
            {
                params.file_anc = args[i + 1];
            }
            if (arg.equals ("--master-board"))
            {
                params.master_board = Integer.parseInt (args[i + 1]);
            }
        }
        return board_id;
    }
}

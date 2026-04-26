package brainflow.examples;

import brainflow.BoardShim;
import brainflow.BrainFlowInputParams;

public class Markers
{

    public static void main (String[] args) throws Exception
    {
        BoardShim.enable_board_logger ();
        BrainFlowInputParams params = new BrainFlowInputParams ();
        int board_id = parse_args (args, params);
        BoardShim board_shim = new BoardShim (board_id, params);

        board_shim.prepare_session ();
        board_shim.start_stream (450000, "file://file_stream.csv:w");
        for (int i = 1; i < 5; i++)
        {
            Thread.sleep (1000);
            board_shim.insert_marker (i);
        }

        board_shim.stop_stream ();
        board_shim.release_session ();
    }

    private static int parse_args (String[] args, BrainFlowInputParams params)
    {
        int board_id = -1;
        for (int i = 0; i < args.length; i++)
        {
            String arg = args[i];
            boolean requires_value = arg.equals ("--ip-address") || arg.equals ("--serial-port")
                    || arg.equals ("--ip-port") || arg.equals ("--ip-protocol")
                    || arg.equals ("--other-info") || arg.equals ("--board-id")
                    || arg.equals ("--timeout") || arg.equals ("--serial-number")
                    || arg.equals ("--file");
            if (requires_value && i + 1 >= args.length)
            {
                throw new IllegalArgumentException ("Missing value for argument: " + arg);
            }
            if (arg.equals ("--ip-address"))
            {
                params.ip_address = args[i + 1];
                i++;
            }
            if (arg.equals ("--serial-port"))
            {
                params.serial_port = args[i + 1];
                i++;
            }
            if (arg.equals ("--ip-port"))
            {
                params.ip_port = Integer.parseInt (args[i + 1]);
                i++;
            }
            if (arg.equals ("--ip-protocol"))
            {
                params.ip_protocol = Integer.parseInt (args[i + 1]);
                i++;
            }
            if (arg.equals ("--other-info"))
            {
                params.other_info = args[i + 1];
                i++;
            }
            if (arg.equals ("--board-id"))
            {
                board_id = Integer.parseInt (args[i + 1]);
                i++;
            }
            if (arg.equals ("--timeout"))
            {
                params.timeout = Integer.parseInt (args[i + 1]);
                i++;
            }
            if (arg.equals ("--serial-number"))
            {
                params.serial_number = args[i + 1];
                i++;
            }
            if (arg.equals ("--file"))
            {
                params.file = args[i + 1];
                i++;
            }
        }
        return board_id;
    }
}

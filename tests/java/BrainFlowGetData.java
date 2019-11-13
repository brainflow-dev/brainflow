import brainflow.*;

import java.util.Arrays;

public class BrainFlowTest
{

    public static void main (String[] args) throws Exception
    {
        BoardShim.enable_board_logger();
        BrainFlowInputParams params = new BrainFlowInputParams();
        int board_id = parse_args(args, params);
        BoardShim board_shim = new BoardShim(board_id, params);
        board_shim.prepare_session();
        board_shim.start_stream(3600);
        BoardShim.log_message(LogLevels.LEVEL_INFO.get_code(), "Start sleeping in the main thread");
        Thread.sleep(5000);
        board_shim.stop_stream();
        System.out.println(board_shim.get_board_data_count());
        int num_rows = BoardShim.get_num_rows(board_id);
        double[][] data = board_shim.get_current_board_data(50);
        for (int i = 0; i < num_rows; i++)
        {
            System.out.println(Arrays.toString(data[i]));
        }
        board_shim.release_session();
        int[] eeg_channels = BoardShim.get_eeg_channels(board_id);
        for (int i = 0; i < eeg_channels.length; i++)
        {
            // just for demo - apply different filters to different eeg channels
            switch (i)
            {
                case 0:
                    DataFilter.perform_lowpass(data[eeg_channels[i]], BoardShim.get_sampling_rate(board_id), 20.0, 4,
                            FilterTypes.BESSEL.get_code(), 0.0);
                    break;
                case 1:
                    DataFilter.perform_highpass(data[eeg_channels[i]], BoardShim.get_sampling_rate(board_id), 20.0, 4,
                            FilterTypes.BUTTERWORTH.get_code(), 0.0);
                    break;
                case 2:
                    DataFilter.perform_bandpass(data[eeg_channels[i]], BoardShim.get_sampling_rate(board_id), 20.0, 5.0,
                            4, FilterTypes.CHEBYSHEV_TYPE_1.get_code(), 1.0);
                    break;
                case 3:
                    DataFilter.perform_bandstop(data[eeg_channels[i]], BoardShim.get_sampling_rate(board_id), 20.0, 5.0,
                            4, FilterTypes.CHEBYSHEV_TYPE_1.get_code(), 1.0);
                    break;
            }
        }
        System.out.println("After signal processing:");
        for (int i = 0; i < num_rows; i++)
        {
            System.out.println(Arrays.toString(data[i]));
        }
    }

    private static int parse_args (String[] args, BrainFlowInputParams params)
    {
        int board_id = -1;
        for (int i = 0; i < args.length; i++)
        {
            if (args[i].equals("--ip-address"))
            {
                params.ip_address = args[i + 1];
            }
            if (args[i].equals("--serial-port"))
            {
                params.serial_port = args[i + 1];
            }
            if (args[i].equals("--ip-port"))
            {
                params.ip_port = Integer.parseInt(args[i + 1]);
            }
            if (args[i].equals("--ip-protocol"))
            {
                params.ip_protocol = Integer.parseInt(args[i + 1]);
            }
            if (args[i].equals("--other-info"))
            {
                params.other_info = args[i + 1];
            }
            if (args[i].equals("--board-id"))
            {
                board_id = Integer.parseInt(args[i + 1]);
            }
        }
        return board_id;
    }
}

namespace brainflow
{
    public class BoardInfoGetter
    {
        public static int get_fs_hz (int board_id)
        {
            if (board_id == (int)BoardIds.CYTON_BOARD)
                return Cyton.fs_hz;
            else
            {
                if (board_id == (int)BoardIds.GANGLION_BOARD)
                    return Ganglion.fs_hz;
                else
                    throw new BrainFlowExceptioin ((int)CustomExitCodes.UNSUPPORTED_BOARD_ERROR);
            }
        }

        public static int get_first_eeg_channel (int board_id)
        {
            if (board_id == (int)BoardIds.CYTON_BOARD)
                return Cyton.first_eeg_channel;
            else
            {
                if (board_id == (int)BoardIds.GANGLION_BOARD)
                    return Ganglion.first_eeg_channel;
                else
                    throw new BrainFlowExceptioin ((int)CustomExitCodes.UNSUPPORTED_BOARD_ERROR);
            }
        }

        public static int get_num_eeg_channels (int board_id)
        {
            if (board_id == (int)BoardIds.CYTON_BOARD)
                return Cyton.num_eeg_channels;
            else
            {
                if (board_id == (int)BoardIds.GANGLION_BOARD)
                    return Ganglion.num_eeg_channels;
                else
                    throw new BrainFlowExceptioin ((int)CustomExitCodes.UNSUPPORTED_BOARD_ERROR);
            }
        }

        public static int get_package_length (int board_id)
        {
            if (board_id == (int)BoardIds.CYTON_BOARD)
                return Cyton.package_length;
            else
            {
                if (board_id == (int)BoardIds.GANGLION_BOARD)
                    return Ganglion.package_length;
                else
                    throw new BrainFlowExceptioin ((int)CustomExitCodes.UNSUPPORTED_BOARD_ERROR);
            }
        }
    }
}

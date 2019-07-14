namespace brainflow
{
    public class BoardInfoGetter
    {
        public static int get_fs_hz (int board_id)
        {
            switch (board_id)
            {
                case (int)BoardIds.CYTON_BOARD:
                    return Cyton.fs_hz;
                case (int)BoardIds.GANGLION_BOARD:
                    return Ganglion.fs_hz;
                case (int)BoardIds.SYNTHETIC_BOARD:
                    return Synthetic.fs_hz;
                case (int)BoardIds.CYTON_DAISY_BOARD:
                    return CytonDaisy.fs_hz;
                default:
                    throw new BrainFlowExceptioin ((int)CustomExitCodes.UNSUPPORTED_BOARD_ERROR);
            }
        }

        public static int get_first_eeg_channel (int board_id)
        {
            switch (board_id)
            {
                case (int)BoardIds.CYTON_BOARD:
                    return Cyton.first_eeg_channel;
                case (int)BoardIds.GANGLION_BOARD:
                    return Ganglion.first_eeg_channel;
                case (int)BoardIds.SYNTHETIC_BOARD:
                    return Synthetic.first_eeg_channel;
                case (int)BoardIds.CYTON_DAISY_BOARD:
                    return CytonDaisy.first_eeg_channel;
                default:
                    throw new BrainFlowExceptioin ((int)CustomExitCodes.UNSUPPORTED_BOARD_ERROR);
            }
        }

        public static int get_num_eeg_channels (int board_id)
        {
            switch (board_id)
            {
                case (int)BoardIds.CYTON_BOARD:
                    return Cyton.num_eeg_channels;
                case (int)BoardIds.GANGLION_BOARD:
                    return Ganglion.num_eeg_channels;
                case (int)BoardIds.SYNTHETIC_BOARD:
                    return Synthetic.num_eeg_channels;
                case (int)BoardIds.CYTON_DAISY_BOARD:
                    return CytonDaisy.num_eeg_channels;
                default:
                    throw new BrainFlowExceptioin ((int)CustomExitCodes.UNSUPPORTED_BOARD_ERROR);
            }
        }

        public static int get_package_length (int board_id)
        {
            switch (board_id)
            {
                case (int)BoardIds.CYTON_BOARD:
                    return Cyton.package_length;
                case (int)BoardIds.GANGLION_BOARD:
                    return Ganglion.package_length;
                case (int)BoardIds.SYNTHETIC_BOARD:
                    return Synthetic.package_length;
                case (int)BoardIds.CYTON_DAISY_BOARD:
                    return CytonDaisy.package_length;
                default:
                    throw new BrainFlowExceptioin ((int)CustomExitCodes.UNSUPPORTED_BOARD_ERROR);
            }
        }
    }
}

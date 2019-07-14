
public class BoardInfoGetter {
	public static int get_fs_hz (int board_id) throws BrainFlowError
	{
		switch (board_id)
		{
			case CYTON.board_id:
				return CYTON.fs_hz;
			case GANGLION.board_id:
				return GANGLION.fs_hz;
			case SYNTHETIC.board_id:
				return SYNTHETIC.fs_hz;
			default:
				throw new  BrainFlowError ("Wrong board_id", ExitCode.UNSUPPORTED_BOARD_ERROR.get_code ());
		}
	}
	
	public static int get_package_length (int board_id) throws BrainFlowError
	{
		switch (board_id)
		{
			case CYTON.board_id:
				return CYTON.package_length;
			case GANGLION.board_id:
				return GANGLION.package_length;
			case SYNTHETIC.board_id:
				return SYNTHETIC.package_length;
			default:
				throw new  BrainFlowError ("Wrong board_id", ExitCode.UNSUPPORTED_BOARD_ERROR.get_code ());
		}
	}
	
	public static int get_num_eeg_channels (int board_id) throws BrainFlowError
	{
		switch (board_id)
		{
			case CYTON.board_id:
				return CYTON.num_eeg_channels;
			case GANGLION.board_id:
				return GANGLION.num_eeg_channels;
			case SYNTHETIC.board_id:
				return SYNTHETIC.num_eeg_channels;
			default:
				throw new  BrainFlowError ("Wrong board_id", ExitCode.UNSUPPORTED_BOARD_ERROR.get_code ());
		}
	}
}


public class BoardInfoGetter {
	public static int get_fs_hz (int board_id) throws BrainFlowError
	{
		if (board_id == CYTON.board_id)
			return CYTON.fs_hz;
		else
		{
			if (board_id == GANGLION.board_id)
				return GANGLION.fs_hz;
			else
				throw new  BrainFlowError ("Wrong board_id", ExitCode.UNSUPPORTED_BOARD_ERROR.get_code ());
		}
	}
	
	public static int get_package_length (int board_id) throws BrainFlowError
	{
		if (board_id == CYTON.board_id)
			return CYTON.package_length;
		else
		{
			if (board_id == GANGLION.board_id)
				return GANGLION.package_length;
			else
				throw new  BrainFlowError ("Wrong board_id", ExitCode.UNSUPPORTED_BOARD_ERROR.get_code ());
		}
	}
	
	public static int get_num_eeg_channels (int board_id) throws BrainFlowError
	{
		if (board_id == CYTON.board_id)
			return CYTON.num_eeg_channels;
		else
		{
			if (board_id == GANGLION.board_id)
				return GANGLION.num_eeg_channels;
			else
				throw new  BrainFlowError ("Wrong board_id", ExitCode.UNSUPPORTED_BOARD_ERROR.get_code ());
		}
	}
}

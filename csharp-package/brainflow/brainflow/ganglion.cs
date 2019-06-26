using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace brainflow
{
    public class Ganglion
    {
        public const int board_id = (int)BoardIds.GANGLION_BOARD;
        public const int fs_hz = 200;
        public const int first_eeg_channel = 1;
        public const int last_eeg_channel = 4;
        public const int num_eeg_channels = 4;
        public const int package_length = 8;
    }
}
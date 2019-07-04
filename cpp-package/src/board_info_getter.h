#ifndef BOARD_INFO_GETTER
#define BOARD_INFO_GETTER

#include "board_controller.h"
#include "brainflow_exception.h"

class CYTON
{
public:
    static const int fs_hz = 250;
    static const int num_eeg_channels = 8;
    static const int package_length = 12;
    static const int first_eeg_channel = 1;
};

class GANGLION
{
public:
    static const int fs_hz = 200;
    static const int num_eeg_channels = 4;
    static const int package_length = 8;
    static const int first_eeg_channel = 1;
};


class BoardInfoGetter
{
public:
    static int get_package_length (int board_id)
    {
        if (board_id == CYTON_BOARD)
            return CYTON::package_length;
        else
        {
            if (board_id == GANGLION_BOARD)
                return GANGLION::package_length;
            else
                throw BrainFlowException ("Unsupported Board Error", UNSUPPORTED_BOARD_ERROR);
        }
    }

    static int get_num_eeg_channels (int board_id)
    {
        if (board_id == CYTON_BOARD)
            return CYTON::num_eeg_channels;
        else
        {
            if (board_id == GANGLION_BOARD)
                return GANGLION::num_eeg_channels;
            else
                throw BrainFlowException ("Unsupported Board Error", UNSUPPORTED_BOARD_ERROR);
        }
    }

    static int get_fs_hz (int board_id)
    {
        if (board_id == CYTON_BOARD)
            return CYTON::fs_hz;
        else
        {
            if (board_id == GANGLION_BOARD)
                return GANGLION::fs_hz;
            else
                throw BrainFlowException ("Unsupported Board Error", UNSUPPORTED_BOARD_ERROR);
        }
    }

    static int get_first_eeg_channel (int board_id)
    {
        if (board_id == CYTON_BOARD)
            return CYTON::first_eeg_channel;
        else
        {
            if (board_id == GANGLION_BOARD)
                return GANGLION::first_eeg_channel;
            else
                throw BrainFlowException ("Unsupported Board Error", UNSUPPORTED_BOARD_ERROR);
        }
    }
};

#endif

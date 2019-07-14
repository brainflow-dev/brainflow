#pragma once

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


class CYTON_DAISY
{
public:
    static const int fs_hz = 125;
    static const int num_eeg_channels = 16;
    static const int package_length = 20;
    static const int first_eeg_channel = 1;
};


class SYNTHETIC
{
public:
    static const int fs_hz = 256;
    static const int num_eeg_channels = 8;
    static const int package_length = 12;
    static const int first_eeg_channel = 1;
};

// todo: move it to low level api during implementation of signal processing in c++
class BoardInfoGetter
{
public:
    static int get_package_length (int board_id)
    {
        switch (board_id)
        {
            case CYTON_BOARD:
                return CYTON::package_length;
            case GANGLION_BOARD:
                return GANGLION::package_length;
            case SYNTHETIC_BOARD:
                return SYNTHETIC::package_length;
            case CYTON_DAISY_BOARD:
                return CYTON_DAISY::package_length;
            default:
                throw BrainFlowException ("Unsupported Board Error", UNSUPPORTED_BOARD_ERROR);
        }
    }

    static int get_num_eeg_channels (int board_id)
    {
        switch (board_id)
        {
            case CYTON_BOARD:
                return CYTON::num_eeg_channels;
            case GANGLION_BOARD:
                return GANGLION::num_eeg_channels;
            case SYNTHETIC_BOARD:
                return SYNTHETIC::num_eeg_channels;
            case CYTON_DAISY_BOARD:
                return CYTON_DAISY::num_eeg_channels;
            default:
                throw BrainFlowException ("Unsupported Board Error", UNSUPPORTED_BOARD_ERROR);
        }
    }

    static int get_fs_hz (int board_id)
    {
        switch (board_id)
        {
            case CYTON_BOARD:
                return CYTON::fs_hz;
            case GANGLION_BOARD:
                return GANGLION::fs_hz;
            case SYNTHETIC_BOARD:
                return SYNTHETIC::fs_hz;
            case CYTON_DAISY_BOARD:
                return CYTON_DAISY::fs_hz;
            default:
                throw BrainFlowException ("Unsupported Board Error", UNSUPPORTED_BOARD_ERROR);
        }
    }

    static int get_first_eeg_channel (int board_id)
    {
        switch (board_id)
        {
            case CYTON_BOARD:
                return CYTON::first_eeg_channel;
            case GANGLION_BOARD:
                return GANGLION::first_eeg_channel;
            case SYNTHETIC_BOARD:
                return SYNTHETIC::first_eeg_channel;
            case CYTON_DAISY_BOARD:
                return CYTON_DAISY::first_eeg_channel;
            default:
                throw BrainFlowException ("Unsupported Board Error", UNSUPPORTED_BOARD_ERROR);
        }
    }
};

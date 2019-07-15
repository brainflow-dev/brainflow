#pragma once

#include "board_info_getter.h"
#include "brainflow_exception.h"

class DataHandler
{
public:
    int start_eeg;
    int stop_eeg;
    int sample_rate;
    int board_id;

    DataHandler (int board_id);

    void filter_lowpass (double **data, int data_count, float cutoff);
    void filter_highpass (double **data, int data_count, float cutoff);
    void filter_bandpass (double **data, int data_count, float min_cutoff, float max_cutoff);
    void remove_dc_offset (double **data, int data_count, float value = 1.0);
    void preprocess_data (double **data, int data_count, float min_cutoff = 1.0,
        float max_cutoff = 50.0, float dc_offset = 1.0);
};

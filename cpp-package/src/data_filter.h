#pragma once

// include it here to allow user include only this single file
#include "brainflow_constants.h"
#include "brainflow_exception.h"
#include "data_handler.h"

class DataFilter
{
public:
    static void perform_lowpass (double *data, int data_len, int sampling_rate, double cutoff,
        int order, int filter_type, double ripple);
    static void perform_highpass (double *data, int data_len, int sampling_rate, double cutoff,
        int order, int filter_type, double ripple);
    static void perform_bandpass (double *data, int data_len, int sampling_rate, double center_freq,
        double band_width, int order, int filter_type, double ripple);
    static void perform_bandstop (double *data, int data_len, int sampling_rate, double center_freq,
        double band_width, int order, int filter_type, double ripple);

    static void write_file (
        double **data, int num_rows, int num_cols, char *file_name, char *file_mode);
    static double **read_file (int *num_rows, int *num_cols, char *file_name);

private:
    static void reshape_data_to_2d (
        int num_rows, int num_cols, double *linear_buffer, double **output_buf);
    static void reshape_data_to_1d (int num_rows, int num_cols, double **buf, double *output_buf);
};

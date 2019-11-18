#pragma once

// include it here to allow user include only this single file
#include "brainflow_constants.h"
#include "brainflow_exception.h"
#include "data_handler.h"


/// DataFilter class to perform signal processing
class DataFilter
{
public:
    /// perform low pass filter in-place
    static void perform_lowpass (double *data, int data_len, int sampling_rate, double cutoff,
        int order, int filter_type, double ripple);
    /// perform high pass filter in-place
    static void perform_highpass (double *data, int data_len, int sampling_rate, double cutoff,
        int order, int filter_type, double ripple);
    /// perform bandpass filter in-place
    static void perform_bandpass (double *data, int data_len, int sampling_rate, double center_freq,
        double band_width, int order, int filter_type, double ripple);
    /// perform bandstop filter in-place
    static void perform_bandstop (double *data, int data_len, int sampling_rate, double center_freq,
        double band_width, int order, int filter_type, double ripple);
    /// perform moving average or moving median filter in-place
    static void perform_rolling_filter (double *data, int data_len, int period, int agg_operation);
    /// perform data downsampling, it just aggregates several data points
    static double *perform_downsampling (
        double *data, int data_len, int period, int agg_operation, int *filtered_size);

    /// write file, in file data will be transposed
    static void write_file (
        double **data, int num_rows, int num_cols, char *file_name, char *file_mode);
    /// read data from file, data will be transposed to original format
    static double **read_file (int *num_rows, int *num_cols, char *file_name);

private:
    static void reshape_data_to_2d (
        int num_rows, int num_cols, double *linear_buffer, double **output_buf);
    static void reshape_data_to_1d (int num_rows, int num_cols, double **buf, double *output_buf);
};

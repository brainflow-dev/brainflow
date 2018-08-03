# Brainflow

Brainflow is a library intended to obtain and analyze EEG and EMG data from OpenBCI boards

It is written on C\C++ and provides a dynamic library which used by different bindings

DLL interface:
```
int prepare_session (int board_id, const char *port_name);
int start_stream (int buffer_size);
int stop_stream ();
int release_session ();
int get_current_board_data (int num_samples, float *data_buf, double *ts_buf, int *returned_samples);
int get_board_data_count (int *result);
int get_board_data (int data_count, float *data_buf, double *ts_buf);
```

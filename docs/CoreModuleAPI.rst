BrainFlow Core API
===================

Core module of BrainFlow is a dynamic library, all bindings just call methods from this dynamic library, it simplifies adding new boards and maintenance

Dynamic library methods :

.. code:: c++

	int prepare_session (int board_id, const char *port_name);
	int start_stream (int buffer_size);
	int stop_stream ();
	int release_session ();
	int get_current_board_data (int num_samples, float *data_buf, double *ts_buf, int *returned_samples);
	int get_board_data_count (int *result);
	int get_board_data (int data_count, float *data_buf, double *ts_buf);
	int set_log_level (int log_level);

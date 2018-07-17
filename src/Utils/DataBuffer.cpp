#include <string.h>
#include <stdlib.h>
#include "DataBuffer.h"

DataBuffer::DataBuffer (size_t num_channels, size_t buffer_size)
{
	this->buffer_size = buffer_size;
	this->num_channels = num_channels;
	data = (float *)malloc (buffer_size * sizeof (float) * num_channels);
	timestamps = (double *)malloc (buffer_size * sizeof (double));
	first_free = first_used = count = 0;
}

DataBuffer::~DataBuffer ()
{
	free (data);
	free (timestamps);
}

void DataBuffer::add_data (double timestamp, float *values)
{
	lock.lock ();
	this->timestamps[first_free] = timestamp;
	memcpy (this->data + first_free * num_channels, values, sizeof (float) * num_channels);
	first_free = next (first_free);
	count++;
	if (first_free == first_used)
	{
		first_used = next (first_used);
		count--;
	}
	lock.unlock ();
}

void DataBuffer::get_chunk (size_t start, size_t size, double *ts_buf, float *data_buf)
{
	if (start + size < buffer_size)
	{
		memcpy (ts_buf, timestamps + start, size * sizeof (double));
		memcpy (data_buf, data + start * num_channels, size * sizeof (float) * num_channels);
	}
	else
	{
		size_t first_half = buffer_size - start;
		size_t second_half = size - first_half;
		memcpy (ts_buf, timestamps + start, first_half * sizeof (double));
		memcpy (data_buf, data + start * num_channels, first_half * sizeof (float) * num_channels);
		memcpy (ts_buf + first_half, timestamps, second_half * sizeof (double));
		memcpy (data_buf + first_half * num_channels, data, second_half * sizeof (float) * num_channels);
	}
}


size_t DataBuffer::get_data (size_t max_count, double *ts_buf, float *data_buf)
{
	lock.lock ();
	size_t result_count = max_count;
	if (result_count > count)
		result_count = count;
	if (result_count)
	{
		get_chunk (first_used, result_count, ts_buf, data_buf);
		first_used = (first_used + result_count) % buffer_size;
		count -= result_count;
	}
	lock.unlock ();
	return result_count;
}

size_t DataBuffer::get_current_data (size_t max_count, double *ts_buf, float *data_buf)
{	
	lock.lock ();
	size_t result_count = max_count;
	if (result_count > count)
		result_count = count;
	if (result_count)
	{
		size_t first_return = (first_used + (count - result_count)) % buffer_size;
		get_chunk (first_return, result_count, ts_buf, data_buf);
	}
	lock.unlock ();
	return result_count;
}

size_t DataBuffer::get_data_count ()
{
	lock.lock ();
	size_t result = this->count;
	lock.unlock ();
	return result;
}

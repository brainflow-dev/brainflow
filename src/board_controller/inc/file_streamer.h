#pragma once

#include <stdio.h>

#include "streamer.h"

#define BRAINFLOW_FILE_NAME_LIMIT 512

class FileStreamer : public Streamer
{

public:
    FileStreamer (const char *file, const char *file_mode, int data_len);
    ~FileStreamer ();

    int init_streamer ();
    void stream_data (double *data);

private:
    char file[BRAINFLOW_FILE_NAME_LIMIT];
    char file_mode[BRAINFLOW_FILE_NAME_LIMIT];
    FILE *fp;
};

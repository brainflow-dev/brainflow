#include <string.h>

#include "brainflow_constants.h"
#include "file_streamer.h"


FileStreamer::FileStreamer (const char *file, const char *file_mode) : Streamer ()
{
    strcpy (this->file, file);
    strcpy (this->file_mode, file_mode);
    fp = NULL;
}

FileStreamer::~FileStreamer ()
{
    if (fp != NULL)
    {
        fclose (fp);
    }
}

int FileStreamer::init_streamer ()
{
    if ((strcmp (file_mode, "w") != 0) && (strcmp (file_mode, "w+") != 0) &&
        (strcmp (file_mode, "a") != 0) && (strcmp (file_mode, "a+") != 0))
    {
        return INVALID_ARGUMENTS_ERROR;
    }
    fp = fopen (file, file_mode);
    if (fp == NULL)
    {
        return GENERAL_ERROR;
    }
    return STATUS_OK;
}

void FileStreamer::stream_data (double *data, int len, double timestamp)
{
    for (int i = 0; i < len; i++)
    {
        fprintf (fp, "%lf,", data[i]);
    }
    fprintf (fp, "%lf\n", timestamp);
}
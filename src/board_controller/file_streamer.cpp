#include <string.h>

#include "brainflow_constants.h"
#include "file_streamer.h"


FileStreamer::FileStreamer (const char *file, const char *file_mode, int data_len)
    : Streamer (data_len)
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
        fp = NULL;
    }
}

int FileStreamer::init_streamer ()
{
    if ((strcmp (file_mode, "w") != 0) && (strcmp (file_mode, "w+") != 0) &&
        (strcmp (file_mode, "a") != 0) && (strcmp (file_mode, "a+") != 0))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    fp = fopen (file, file_mode);
    if (fp == NULL)
    {
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void FileStreamer::stream_data (double *data)
{
    for (int i = 0; i < len - 1; i++)
    {
        fprintf (fp, "%lf\t", data[i]);
    }
    fprintf (fp, "%lf\n", data[len - 1]);
}

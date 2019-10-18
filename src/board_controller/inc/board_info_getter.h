#pragma once

#ifdef _WIN32
#define SHARED_EXPORT __declspec(dllexport)
#define CALLING_CONVENTION __cdecl
#else
#define SHARED_EXPORT
#define CALLING_CONVENTION
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    // data and board desc methods
    SHARED_EXPORT int CALLING_CONVENTION get_sampling_rate (int board_id, int *sampling_rate);
    SHARED_EXPORT int CALLING_CONVENTION get_package_num_channel (
        int board_id, int *package_num_channel);
    SHARED_EXPORT int CALLING_CONVENTION get_timestamp_channel (
        int board_id, int *timestamp_channel);
    SHARED_EXPORT int CALLING_CONVENTION get_num_rows (int board_id, int *num_rows);
    SHARED_EXPORT int CALLING_CONVENTION get_eeg_channels (
        int board_id, int *eeg_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_emg_channels (
        int board_id, int *emg_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_ecg_channels (
        int board_id, int *ecg_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_eog_channels (
        int board_id, int *eog_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_ppg_channels (
        int board_id, int *ppg_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_eda_channels (
        int board_id, int *eda_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_accel_channels (
        int board_id, int *accel_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_gyro_channels (
        int board_id, int *gyro_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_other_channels (
        int board_id, int *other_channels, int *len);

#ifdef __cplusplus
}
#endif

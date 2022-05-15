#pragma once

#include "shared_export.h"

#ifdef __cplusplus
extern "C"
{
#endif
    // data and board desc methods
    SHARED_EXPORT int CALLING_CONVENTION get_board_descr (
        int board_id, char *board_descr, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_sampling_rate (int board_id, int *sampling_rate);
    SHARED_EXPORT int CALLING_CONVENTION get_package_num_channel (
        int board_id, int *package_num_channel);
    SHARED_EXPORT int CALLING_CONVENTION get_timestamp_channel (
        int board_id, int *timestamp_channel);
    SHARED_EXPORT int CALLING_CONVENTION get_marker_channel (int board_id, int *marker_channel);
    SHARED_EXPORT int CALLING_CONVENTION get_battery_channel (int board_id, int *battery_channel);
    SHARED_EXPORT int CALLING_CONVENTION get_num_rows (int board_id, int *num_rows);
    SHARED_EXPORT int CALLING_CONVENTION get_eeg_names (int board_id, char *eeg_names, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_exg_channels (
        int board_id, int *exg_channels, int *len);
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
    SHARED_EXPORT int CALLING_CONVENTION get_analog_channels (
        int board_id, int *analog_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_gyro_channels (
        int board_id, int *gyro_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_other_channels (
        int board_id, int *other_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_temperature_channels (
        int board_id, int *temperature_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_resistance_channels (
        int board_id, int *resistance_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_device_name (int board_id, char *name, int *len);

#ifdef __cplusplus
}
#endif

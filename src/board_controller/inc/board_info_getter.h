#pragma once

#include "shared_export.h"

#ifdef __cplusplus
extern "C"
{
#endif
    // data and board desc methods
    SHARED_EXPORT int CALLING_CONVENTION get_board_descr (
        int board_id, int preset, char *board_descr, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_sampling_rate (
        int board_id, int preset, int *sampling_rate);
    SHARED_EXPORT int CALLING_CONVENTION get_package_num_channel (
        int board_id, int preset, int *package_num_channel);
    SHARED_EXPORT int CALLING_CONVENTION get_timestamp_channel (
        int board_id, int preset, int *timestamp_channel);
    SHARED_EXPORT int CALLING_CONVENTION get_marker_channel (
        int board_id, int preset, int *marker_channel);
    SHARED_EXPORT int CALLING_CONVENTION get_battery_channel (
        int board_id, int preset, int *battery_channel);
    SHARED_EXPORT int CALLING_CONVENTION get_num_rows (int board_id, int preset, int *num_rows);
    SHARED_EXPORT int CALLING_CONVENTION get_eeg_names (
        int board_id, int preset, char *eeg_names, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_exg_channels (
        int board_id, int preset, int *exg_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_eeg_channels (
        int board_id, int preset, int *eeg_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_emg_channels (
        int board_id, int preset, int *emg_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_ecg_channels (
        int board_id, int preset, int *ecg_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_eog_channels (
        int board_id, int preset, int *eog_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_ppg_channels (
        int board_id, int preset, int *ppg_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_eda_channels (
        int board_id, int preset, int *eda_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_accel_channels (
        int board_id, int preset, int *accel_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_analog_channels (
        int board_id, int preset, int *analog_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_gyro_channels (
        int board_id, int preset, int *gyro_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_other_channels (
        int board_id, int preset, int *other_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_temperature_channels (
        int board_id, int preset, int *temperature_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_resistance_channels (
        int board_id, int preset, int *resistance_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_magnetometer_channels (
        int board_id, int preset, int *magnetometer_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_device_name (
        int board_id, int preset, char *name, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_board_presets (int board_id, int *presets, int *len);

#ifdef __cplusplus
}
#endif

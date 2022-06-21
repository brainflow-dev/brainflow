#pragma once

#include "shared_export.h"

#ifdef __cplusplus
extern "C"
{
#endif
    // data and board desc methods
    SHARED_EXPORT int CALLING_CONVENTION get_board_descr (
        int board_id, const char *preset, char *board_descr, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_sampling_rate (
        int board_id, const char *preset, int *sampling_rate);
    SHARED_EXPORT int CALLING_CONVENTION get_package_num_channel (
        int board_id, const char *preset, int *package_num_channel);
    SHARED_EXPORT int CALLING_CONVENTION get_timestamp_channel (
        int board_id, const char *preset, int *timestamp_channel);
    SHARED_EXPORT int CALLING_CONVENTION get_marker_channel (
        int board_id, const char *preset, int *marker_channel);
    SHARED_EXPORT int CALLING_CONVENTION get_battery_channel (
        int board_id, const char *preset, int *battery_channel);
    SHARED_EXPORT int CALLING_CONVENTION get_num_rows (
        int board_id, const char *preset, int *num_rows);
    SHARED_EXPORT int CALLING_CONVENTION get_eeg_names (
        int board_id, const char *preset, char *eeg_names, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_exg_channels (
        int board_id, const char *preset, int *exg_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_eeg_channels (
        int board_id, const char *preset, int *eeg_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_emg_channels (
        int board_id, const char *preset, int *emg_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_ecg_channels (
        int board_id, const char *preset, int *ecg_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_eog_channels (
        int board_id, const char *preset, int *eog_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_ppg_channels (
        int board_id, const char *preset, int *ppg_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_eda_channels (
        int board_id, const char *preset, int *eda_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_accel_channels (
        int board_id, const char *preset, int *accel_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_analog_channels (
        int board_id, const char *preset, int *analog_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_gyro_channels (
        int board_id, const char *preset, int *gyro_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_other_channels (
        int board_id, const char *preset, int *other_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_temperature_channels (
        int board_id, const char *preset, int *temperature_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_resistance_channels (
        int board_id, const char *preset, int *resistance_channels, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_device_name (
        int board_id, const char *preset, char *name, int *len);
    SHARED_EXPORT int CALLING_CONVENTION get_board_presets (
        int board_id, int max_len, char *presets, int *len);

#ifdef __cplusplus
}
#endif

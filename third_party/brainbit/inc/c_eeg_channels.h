#ifndef EEG_EXTENSIONS_CHANNELS_H
#define EEG_EXTENSIONS_CHANNELS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "cchannels.h"
#include "cdevice.h"
#include "lib_export.h"

    typedef struct _EegDoubleChannel EegDoubleChannel;
    typedef struct _EegIndexChannel EegIndexChannel;
    typedef struct _EegArtifactChannel EegArtifactChannel;
    typedef struct _EmotionalStateChannel EmotionalStateChannel;
    typedef struct _MeditationAnalyzer MeditationAnalyzer;

    SDK_SHARED EegDoubleChannel *create_EegDoubleChannel (Device *device_ptr);
    SDK_SHARED EegDoubleChannel *create_EegDoubleChannel_info (
        Device *device_ptr, ChannelInfo info);
    SDK_SHARED EegDoubleChannel *create_EegDoubleChannel_SignalChannel (DoubleChannel *channel);

    typedef enum _ArtifactType
    {
        ArtifactTypeNone,
        ArtifactTypeNoise,
        ArtifactTypeBlink,
        ArtifactTypeBrux
    } ArtifactType;
    typedef enum _SourceChannel : unsigned int
    {
        ArtifactChannelT3 = 0b00000001,
        ArtifactChannelT4 = 0b00000010,
        ArtifactChannelO1 = 0b00000100,
        ArtifactChannelO2 = 0b00001000
    } SourceChannel;

    typedef struct _ArtifactZone
    {
        double time;
        double duration;
        ArtifactType type;
        SourceChannel source_channels_flags;
    } ArtifactZone;

    SDK_SHARED EegArtifactChannel *create_EegArtifactChannel_eeg_channels (
        EegDoubleChannel *t3, EegDoubleChannel *t4, EegDoubleChannel *o1, EegDoubleChannel *o2);
    SDK_SHARED int EegArtifactChannel_read_data (EegArtifactChannel *channel, size_t offset,
        size_t length, ArtifactZone *out_buffer, size_t buffer_size, size_t *samples_read);
    SDK_SHARED int EegArtifactChannel_get_buffer_size (
        EegArtifactChannel *channel, size_t *out_buffer_size);
    SDK_SHARED int delete_SourceChannel (SourceChannel *channels);

    typedef struct _EegIndexValues
    {
        double AlphaRate;
        double BetaRate;
        double DeltaRate;
        double ThetaRate;
    } EegIndexValues;

    typedef enum _EegIndexMode
    {
        EegIndexModeLeftSide,
        EegIndexModeRightSide,
        EegIndexModeArtifacts
    } EegIndexMode;

    SDK_SHARED EegIndexChannel *create_EegIndexChannel (
        EegDoubleChannel *t3, EegDoubleChannel *t4, EegDoubleChannel *o1, EegDoubleChannel *o2);
    SDK_SHARED int EegIndexChannel_read_data (EegIndexChannel *channel, size_t offset,
        size_t length, EegIndexValues *out_buffer, size_t buffer_size, size_t *samples_read);
    SDK_SHARED int EegIndexChannel_get_buffer_size (
        EegIndexChannel *channel, size_t *out_buffer_size);
    SDK_SHARED int EegIndexChannel_set_delay (EegIndexChannel *channel, double delay_seconds);
    SDK_SHARED int EegIndexChannel_set_weight_coefficients (
        EegIndexChannel *channel, double alpha, double beta, double delta, double theta);
    SDK_SHARED int EegIndexChannel_get_mode (EegIndexChannel *channel, EegIndexMode *out_mode);
    SDK_SHARED int EegIndexChannel_get_base_power_left (
        EegIndexChannel *channel, double *out_power);
    SDK_SHARED int EegIndexChannel_get_base_power_right (
        EegIndexChannel *channel, double *out_power);

    typedef struct _BaseEmotionalValue
    {
        double Alpha;
        double Beta;
    } BaseEmotionalValue;

    typedef struct _EmotionalState
    {
        double RelaxationRate;
        double ConcentrationRate;
    } EmotionalState;

    SDK_SHARED EmotionalStateChannel *create_EmotionalStateChannel (EegIndexChannel *index_channel);
    SDK_SHARED int EmotionalStateChannel_read_data (EmotionalStateChannel *channel, size_t offset,
        size_t length, EmotionalState *out_buffer, size_t buffer_size, size_t *samples_read);
    SDK_SHARED int EmotionalStateChannel_get_buffer_size (
        EmotionalStateChannel *channel, size_t *out_buffer_size);
    SDK_SHARED int EmotionalStateChannel_get_base_state_values (
        EmotionalStateChannel *channel, BaseEmotionalValue *out_base_value);

    typedef void *MeditationLevelListener;
    typedef void *MeditationLevelProgressListener;

    SDK_SHARED MeditationAnalyzer *create_MeditationAnalyzer (EegIndexChannel *index_channel);
    SDK_SHARED void MeditationAnalyzer_delete (MeditationAnalyzer *analyzer);
    SDK_SHARED int MeditationAnalyzer_level (MeditationAnalyzer *analyzer, int *out_level);
    SDK_SHARED int MeditationAnalyzer_level_progress (
        MeditationAnalyzer *analyzer, double *out_level_progress);
    SDK_SHARED int MeditationAnalyzer_set_level_changed_callback (MeditationAnalyzer *,
        void (*) (MeditationAnalyzer *, int, void *), MeditationLevelListener *, void *user_data);
    SDK_SHARED int MeditationAnalyzer_set_level_progress_changed_callback (MeditationAnalyzer *,
        void (*) (MeditationAnalyzer *, double, void *), MeditationLevelProgressListener *,
        void *user_data);
    SDK_SHARED void free_MeditationLevelListener (MeditationLevelListener *);
    SDK_SHARED void free_MeditationLevelProgressListener (MeditationLevelProgressListener *);

#ifdef __cplusplus
}
#endif

#endif

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

    typedef void *MeditationLevelListener;
    typedef void *MeditationLevelProgressListener;

#ifdef __cplusplus
}
#endif

#endif

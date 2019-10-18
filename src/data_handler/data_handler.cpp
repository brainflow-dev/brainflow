#include "data_handler.h"

#include "DspFilters/Dsp.h"


int perform_lowpass (double *data, int data_len, int sampling_rate, double cutoff, int order,
    int filter_type, double ripple)
{
    int numSamples = 2000;
    double *filter_data[1];
    filter_data[0] = data;

    Dsp::Filter *f = NULL;
    if ((order < 1) || (order > MAX_FILTER_ORDER) || (!data))
    {
        return GENERAL_ERROR;
    }
    switch (filter_type)
    {
        case BUTTERWORTH:
            // "1024" is the number of samples over which to fade parameter changes
            f = new Dsp::SmoothedFilterDesign<Dsp::Butterworth::Design::LowPass<MAX_FILTER_ORDER>,
                1, Dsp::DirectFormII> (1024);
            break;
        case CHEBYSHEV_TYPE_1:
            f = new Dsp::SmoothedFilterDesign<Dsp::ChebyshevI::Design::LowPass<MAX_FILTER_ORDER>, 1,
                Dsp::DirectFormII> (1024);
            break;
        case BESSEL:
            f = new Dsp::SmoothedFilterDesign<Dsp::Bessel::Design::LowPass<MAX_FILTER_ORDER>, 1,
                Dsp::DirectFormII> (1024);
            break;
        default:
            return INVALID_ARGUMENTS_ERROR;
    }

    Dsp::Params params;
    params[0] = sampling_rate; // sample rate
    params[1] = order;         // order
    params[2] = cutoff;        // cutoff
    if (filter_type == CHEBYSHEV_TYPE_1)
    {
        params[3] = ripple; // ripple
    }
    f->setParams (params);
    f->process (data_len, filter_data);
    delete f;

    return STATUS_OK;
}

int perform_highpass (double *data, int data_len, int sampling_rate, double cutoff, int order,
    int filter_type, double ripple)
{
    Dsp::Filter *f = NULL;
    double *filter_data[1];
    filter_data[0] = data;

    if ((order < 1) || (order > MAX_FILTER_ORDER) || (!data))
    {
        return INVALID_ARGUMENTS_ERROR;
    }
    switch (filter_type)
    {
        case BUTTERWORTH:
            // "1024" is the number of samples over which to fade parameter changes
            f = new Dsp::SmoothedFilterDesign<Dsp::Butterworth::Design::HighPass<MAX_FILTER_ORDER>,
                1, Dsp::DirectFormII> (1024);
            break;
        case CHEBYSHEV_TYPE_1:
            f = new Dsp::SmoothedFilterDesign<Dsp::ChebyshevI::Design::HighPass<MAX_FILTER_ORDER>,
                1, Dsp::DirectFormII> (1024);
            break;
        case BESSEL:
            f = new Dsp::SmoothedFilterDesign<Dsp::Bessel::Design::HighPass<MAX_FILTER_ORDER>, 1,
                Dsp::DirectFormII> (1024);
            break;
        default:
            return INVALID_ARGUMENTS_ERROR;
    }
    Dsp::Params params;
    params[0] = sampling_rate; // sample rate
    params[1] = order;         // order
    params[2] = cutoff;        // cutoff
    if (filter_type == CHEBYSHEV_TYPE_1)
    {
        params[3] = ripple; // ripple
    }
    f->setParams (params);
    f->process (data_len, filter_data);
    delete f;

    return STATUS_OK;
}

int perform_bandpass (double *data, int data_len, int sampling_rate, double center_freq,
    double band_width, int order, int filter_type, double ripple)
{
    Dsp::Filter *f = NULL;
    double *filter_data[1];
    filter_data[0] = data;

    if ((order < 1) || (order > MAX_FILTER_ORDER) || (!data))
    {
        return INVALID_ARGUMENTS_ERROR;
    }
    switch (filter_type)
    {
        case BUTTERWORTH:
            // "1024" is the number of samples over which to fade parameter changes
            f = new Dsp::SmoothedFilterDesign<Dsp::Butterworth::Design::BandPass<MAX_FILTER_ORDER>,
                1, Dsp::DirectFormII> (1024);
            break;
        case CHEBYSHEV_TYPE_1:
            f = new Dsp::SmoothedFilterDesign<Dsp::ChebyshevI::Design::BandPass<MAX_FILTER_ORDER>,
                1, Dsp::DirectFormII> (1024);
            break;
        case BESSEL:
            f = new Dsp::SmoothedFilterDesign<Dsp::Bessel::Design::BandPass<MAX_FILTER_ORDER>, 1,
                Dsp::DirectFormII> (1024);
            break;
        default:
            return INVALID_ARGUMENTS_ERROR;
    }

    Dsp::Params params;
    params[0] = sampling_rate; // sample rate
    params[1] = order;         // order
    params[2] = center_freq;   // center freq
    params[3] = band_width;
    if (filter_type == CHEBYSHEV_TYPE_1)
    {
        params[4] = ripple; // ripple
    }
    f->setParams (params);

    f->process (data_len, filter_data);
    delete f;

    return STATUS_OK;
}

int perform_bandstop (double *data, int data_len, int sampling_rate, double center_freq,
    double band_width, int order, int filter_type, double ripple)
{
    Dsp::Filter *f = NULL;
    double *filter_data[1];
    filter_data[0] = data;

    if ((order < 1) || (order > MAX_FILTER_ORDER) || (!data))
    {
        return INVALID_ARGUMENTS_ERROR;
    }
    switch (filter_type)
    {
        case BUTTERWORTH:
            // "1024" is the number of samples over which to fade parameter changes
            f = new Dsp::SmoothedFilterDesign<Dsp::Butterworth::Design::BandStop<MAX_FILTER_ORDER>,
                1, Dsp::DirectFormII> (1024);
            break;
        case CHEBYSHEV_TYPE_1:
            f = new Dsp::SmoothedFilterDesign<Dsp::ChebyshevI::Design::BandStop<MAX_FILTER_ORDER>,
                1, Dsp::DirectFormII> (1024);
            break;
        case BESSEL:
            f = new Dsp::SmoothedFilterDesign<Dsp::Bessel::Design::BandStop<MAX_FILTER_ORDER>, 1,
                Dsp::DirectFormII> (1024);
            break;
        default:
            return INVALID_ARGUMENTS_ERROR;
    }

    Dsp::Params params;
    params[0] = sampling_rate; // sample rate
    params[1] = order;         // order
    params[2] = center_freq;   // center freq
    params[3] = band_width;
    if (filter_type == CHEBYSHEV_TYPE_1)
    {
        params[4] = ripple; // ripple
    }
    f->setParams (params);
    f->process (data_len, filter_data);
    delete f;

    return STATUS_OK;
}
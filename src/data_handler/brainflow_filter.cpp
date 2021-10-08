#include "brainflow_filter.h"
#include "brainflow_constants.h"
#include "brainflow_exception.h"
#include "data_handler.h"
#include "downsample_operators.h"
#include "rolling_filter.h"

#include "DspFilters/Dsp.h"

#include "spdlog/spdlog.h"

#define LOGGER_NAME "data_logger"
#define MAX_FILTER_ORDER 8

extern std::shared_ptr<spdlog::logger> data_logger;

static std::vector<std::unique_ptr<brainflow_filter>> filters;
static std::mutex filters_mutex;

brainflow_filter::~brainflow_filter ()
{
}

class brainflow_dsp_filter_base : public brainflow_filter
{
public:
    brainflow_dsp_filter_base () : brainflow_filter {} {};
    ~brainflow_dsp_filter_base () {};

    virtual void setParams (const Dsp::Params &parameters) = 0;
};

template <typename FILTER_TYPE>
class brainflow_dsp_filter : public brainflow_dsp_filter_base
{
public:
    brainflow_dsp_filter () : brainflow_dsp_filter_base {}, filter {} {};
    ~brainflow_dsp_filter () {};

    int process (double *data, int data_len) override
    {
        double *filter_data[1] = {data};

        filter.process (data_len, filter_data);
        return data_len;
    }
    void setParams (const Dsp::Params &parameters) override
    {
        filter.setParams (parameters);
    }

private:
    FILTER_TYPE filter;
};

template <typename FILTER_TYPE>
class brainflow_rolling_filter : public brainflow_filter
{
public:
    brainflow_rolling_filter (int period) : brainflow_filter {}, filter {period} {};
    ~brainflow_rolling_filter () {};

    int process (double *data, int data_len) override
    {
        for (int i = 0; i < data_len; i++)
        {
            filter.add_data (data[i]);
            data[i] = filter.get_value ();
        }
        return data_len;
    }

private:
    FILTER_TYPE filter;
};

class brainflow_unity_filter : public brainflow_filter
{
public:
    brainflow_unity_filter (int period) : brainflow_filter {} {};
    ~brainflow_unity_filter () {};

    int process (double *data, int data_len) override
    {
        return data_len;
    }
};

template <typename FILTER_TYPE>
class brainflow_downsampling_filter : public brainflow_filter
{
public:
    brainflow_downsampling_filter (int period)
        : brainflow_filter {}, filter {period}, period {period}, sample_count {0} {};
    ~brainflow_downsampling_filter () {};

    int process (double *data, int data_len) override
    {
        int out_i = 0;
        for (int i = 0; i < data_len; i++)
        {
            filter.add_data (data[i]);
            if (++sample_count == period)
            {
                sample_count = 0;
                data[out_i] = filter.get_value ();
                ++out_i;
            }
        }
        return out_i;
    }

private:
    FILTER_TYPE filter;
    const int period;
    int sample_count;
};

std::unique_ptr<brainflow_filter> create_lowpass_filter (
    int sampling_rate, double cutoff, int order, int filter_type, double ripple)
{
    if ((order < 1) || (order > MAX_FILTER_ORDER))
    {
        data_logger->error ("Order must be from 1-8. Order:{}", order);
        throw BrainFlowException {"Order must be from 1-8",
            static_cast<int> (BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR)};
    }

    brainflow_dsp_filter_base *filter = nullptr;

    switch (static_cast<FilterTypes> (filter_type))
    {
        case FilterTypes::BUTTERWORTH:
            filter = new brainflow_dsp_filter<
                Dsp::FilterDesign<Dsp::Butterworth::Design::LowPass<MAX_FILTER_ORDER>, 1>> ();
            break;
        case FilterTypes::CHEBYSHEV_TYPE_1:
            filter = new brainflow_dsp_filter<
                Dsp::FilterDesign<Dsp::ChebyshevI::Design::LowPass<MAX_FILTER_ORDER>, 1>> ();
            break;
        case FilterTypes::BESSEL:
            filter = new brainflow_dsp_filter<
                Dsp::FilterDesign<Dsp::Bessel::Design::LowPass<MAX_FILTER_ORDER>, 1>> ();
            break;
        default:
            data_logger->error ("Filter type {} is Invalid", filter_type);
            throw BrainFlowException {"Invalid filter type",
                static_cast<int> (BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR)};
    }

    Dsp::Params params;
    params[0] = sampling_rate;
    params[1] = order;
    params[2] = cutoff;
    if (filter_type == (int)FilterTypes::CHEBYSHEV_TYPE_1)
    {
        params[3] = ripple;
    }
    filter->setParams (params);

    return std::unique_ptr<brainflow_filter> {filter};
}

std::unique_ptr<brainflow_filter> create_highpass_filter (
    int sampling_rate, double cutoff, int order, int filter_type, double ripple)
{
    if ((order < 1) || (order > MAX_FILTER_ORDER))
    {
        data_logger->error ("Order must be from 1-8. Order:{}", order);
        throw BrainFlowException {"Order must be from 1-8",
            static_cast<int> (BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR)};
    }

    brainflow_dsp_filter_base *filter = nullptr;

    switch (static_cast<FilterTypes> (filter_type))
    {
        case FilterTypes::BUTTERWORTH:
            filter = new brainflow_dsp_filter<
                Dsp::FilterDesign<Dsp::Butterworth::Design::HighPass<MAX_FILTER_ORDER>, 1>> ();
            break;
        case FilterTypes::CHEBYSHEV_TYPE_1:
            filter = new brainflow_dsp_filter<
                Dsp::FilterDesign<Dsp::ChebyshevI::Design::HighPass<MAX_FILTER_ORDER>, 1>> ();
            break;
        case FilterTypes::BESSEL:
            filter = new brainflow_dsp_filter<
                Dsp::FilterDesign<Dsp::Bessel::Design::HighPass<MAX_FILTER_ORDER>, 1>> ();
            break;
        default:
            data_logger->error ("Filter type {} is Invalid", filter_type);
            throw BrainFlowException {"Invalid filter type",
                static_cast<int> (BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR)};
    }
    Dsp::Params params;
    params[0] = sampling_rate;
    params[1] = order;
    params[2] = cutoff;
    if (filter_type == (int)FilterTypes::CHEBYSHEV_TYPE_1)
    {
        params[3] = ripple;
    }
    filter->setParams (params);

    return std::unique_ptr<brainflow_filter> {filter};
}

std::unique_ptr<brainflow_filter> create_bandpass_filter (int sampling_rate, double center_freq,
    double band_width, int order, int filter_type, double ripple)
{
    if ((order < 1) || (order > MAX_FILTER_ORDER))
    {
        data_logger->error ("Order must be from 1-8. Order:{}", order);
        throw BrainFlowException {"Order must be from 1-8",
            static_cast<int> (BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR)};
    }

    brainflow_dsp_filter_base *filter = nullptr;

    switch (static_cast<FilterTypes> (filter_type))
    {
        case FilterTypes::BUTTERWORTH:
            filter = new brainflow_dsp_filter<
                Dsp::FilterDesign<Dsp::Butterworth::Design::BandPass<MAX_FILTER_ORDER>, 1>> ();
            break;
        case FilterTypes::CHEBYSHEV_TYPE_1:
            filter = new brainflow_dsp_filter<
                Dsp::FilterDesign<Dsp::ChebyshevI::Design::BandPass<MAX_FILTER_ORDER>, 1>> ();
            break;
        case FilterTypes::BESSEL:
            filter = new brainflow_dsp_filter<
                Dsp::FilterDesign<Dsp::Bessel::Design::BandPass<MAX_FILTER_ORDER>, 1>> ();
            break;
        default:
            data_logger->error ("Filter type {} is Invalid. ", filter_type);
            throw BrainFlowException {"Invalid filter type",
                static_cast<int> (BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR)};
    }

    Dsp::Params params;
    params[0] = sampling_rate;
    params[1] = order;
    params[2] = center_freq;
    params[3] = band_width;
    if (filter_type == (int)FilterTypes::CHEBYSHEV_TYPE_1)
    {
        params[4] = ripple;
    }
    filter->setParams (params);

    return std::unique_ptr<brainflow_filter> {filter};
}

std::unique_ptr<brainflow_filter> create_bandstop_filter (int sampling_rate, double center_freq,
    double band_width, int order, int filter_type, double ripple)
{
    if ((order < 1) || (order > MAX_FILTER_ORDER))
    {
        data_logger->error ("Order must be from 1-8. Order:{}", order);
        throw BrainFlowException {"Order must be from 1-8",
            static_cast<int> (BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR)};
    }

    brainflow_dsp_filter_base *filter = nullptr;

    switch (static_cast<FilterTypes> (filter_type))
    {
        case FilterTypes::BUTTERWORTH:
            filter = new brainflow_dsp_filter<
                Dsp::FilterDesign<Dsp::Butterworth::Design::BandStop<MAX_FILTER_ORDER>, 1>> ();
            break;
        case FilterTypes::CHEBYSHEV_TYPE_1:
            filter = new brainflow_dsp_filter<
                Dsp::FilterDesign<Dsp::ChebyshevI::Design::BandStop<MAX_FILTER_ORDER>, 1>> ();
            break;
        case FilterTypes::BESSEL:
            filter = new brainflow_dsp_filter<
                Dsp::FilterDesign<Dsp::Bessel::Design::BandStop<MAX_FILTER_ORDER>, 1>> ();
            break;
        default:
            data_logger->error ("Filter type {} is Invalid", filter_type);
            throw BrainFlowException {"Invalid filter type",
                static_cast<int> (BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR)};
    }

    Dsp::Params params;
    params[0] = sampling_rate;
    params[1] = order;
    params[2] = center_freq;
    params[3] = band_width;
    if (filter_type == (int)FilterTypes::CHEBYSHEV_TYPE_1)
    {
        params[4] = ripple;
    }
    filter->setParams (params);

    return std::unique_ptr<brainflow_filter> {filter};
}

std::unique_ptr<brainflow_filter> create_environmental_noise_filter (
    int sampling_rate, int noise_type)
{
    std::unique_ptr<brainflow_filter> filter;
    if (sampling_rate < 1)
    {
        throw BrainFlowException {
            "Invalid sample rate", static_cast<int> (BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR)};
    }

    int res = static_cast<int> (BrainFlowExitCodes::STATUS_OK);

    switch (static_cast<NoiseTypes> (noise_type))
    {
        case NoiseTypes::FIFTY:
            filter = create_bandstop_filter (
                sampling_rate, 50.0, 4.0, 4, static_cast<int> (FilterTypes::BUTTERWORTH), 0.0);
            break;
        case NoiseTypes::SIXTY:
            filter = create_bandstop_filter (
                sampling_rate, 60.0, 4.0, 4, static_cast<int> (FilterTypes::BUTTERWORTH), 0.0);
            break;
        default:
            data_logger->error ("Invalid noise type");
            throw BrainFlowException {"Invalid noise type",
                static_cast<int> (BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR)};
    }
    if (res != (int)BrainFlowExitCodes::STATUS_OK || !filter)
    {
        throw BrainFlowException {"Failed to create filter",
            static_cast<int> (BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR)};
    }

    return filter;
}

std::unique_ptr<brainflow_filter> create_rolling_filter (int period, int agg_operation)
{
    if ((period <= 0))
    {
        data_logger->error ("Period must be >= 0. Period:{}", period);
        throw BrainFlowException {
            "Invalid period", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR};
    }

    brainflow_filter *filter = nullptr;

    switch (static_cast<AggOperations> (agg_operation))
    {
        case AggOperations::MEAN:
            filter = new brainflow_rolling_filter<RollingAverage<double>> (period);
            break;
        case AggOperations::MEDIAN:
            filter = new brainflow_rolling_filter<RollingMedian<double>> (period);
            break;
        case AggOperations::EACH:
            filter = new brainflow_unity_filter (period);
            break;
        default:
            data_logger->error ("Invalid aggregate opteration:{}", agg_operation);
            throw BrainFlowException {
                "Invalid aggregate opteration", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR};
    }

    return std::unique_ptr<brainflow_filter> {filter};
}

std::unique_ptr<brainflow_filter> create_downsampling_filter (int period, int agg_operation)
{
    if ((period <= 0))
    {
        data_logger->error ("Period must be >= 0. Period:{}", period);
        throw BrainFlowException {
            "Invalid period", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR};
    }

    brainflow_filter *filter = nullptr;

    // TODO: replace RollingAverage and RollingMedian with simple array buffer
    switch (static_cast<AggOperations> (agg_operation))
    {
        case AggOperations::MEAN:
            filter = new brainflow_downsampling_filter<RollingAverage<double>> (period);
            break;
        case AggOperations::MEDIAN:
            filter = new brainflow_downsampling_filter<RollingMedian<double>> (period);
            break;
        default:
            data_logger->error ("Invalid aggregate opteration:{}", agg_operation);
            throw BrainFlowException {
                "Invalid aggregate opteration", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR};
    }

    return std::unique_ptr<brainflow_filter> {filter};
}

int create_lowpass (
    int *filter_id, int sampling_rate, double cutoff, int order, int filter_type, double ripple)
{
    int result = static_cast<int> (BrainFlowExitCodes::STATUS_OK);
    try
    {
        const std::lock_guard<std::mutex> filters_lock (filters_mutex);

        const int id = static_cast<int> (filters.size ());
        filters.push_back (
            create_lowpass_filter (sampling_rate, cutoff, order, filter_type, ripple));
        *filter_id = id;
    }
    catch (const BrainFlowException &e)
    {
        result = e.exit_code;
    }
    catch (...)
    {
        result = static_cast<int> (BrainFlowExitCodes::GENERAL_ERROR);
    }
    return result;
}

int create_highpass (
    int *filter_id, int sampling_rate, double cutoff, int order, int filter_type, double ripple)
{
    int result = static_cast<int> (BrainFlowExitCodes::STATUS_OK);
    try
    {
        const std::lock_guard<std::mutex> filters_lock (filters_mutex);

        const int id = static_cast<int> (filters.size ());
        filters.push_back (
            create_highpass_filter (sampling_rate, cutoff, order, filter_type, ripple));
        *filter_id = id;
    }
    catch (const BrainFlowException &e)
    {
        result = e.exit_code;
    }
    catch (...)
    {
        result = static_cast<int> (BrainFlowExitCodes::GENERAL_ERROR);
    }
    return result;
}

int create_bandpass (int *filter_id, int sampling_rate, double center_freq, double band_width,
    int order, int filter_type, double ripple)
{
    int result = static_cast<int> (BrainFlowExitCodes::STATUS_OK);
    try
    {
        const std::lock_guard<std::mutex> filters_lock (filters_mutex);

        const int id = static_cast<int> (filters.size ());
        filters.push_back (create_bandpass_filter (
            sampling_rate, center_freq, band_width, order, filter_type, ripple));
        *filter_id = id;
    }
    catch (const BrainFlowException &e)
    {
        result = e.exit_code;
    }
    catch (...)
    {
        result = static_cast<int> (BrainFlowExitCodes::GENERAL_ERROR);
    }
    return result;
}

int create_bandstop (int *filter_id, int sampling_rate, double center_freq, double band_width,
    int order, int filter_type, double ripple)
{
    int result = static_cast<int> (BrainFlowExitCodes::STATUS_OK);
    try
    {
        const std::lock_guard<std::mutex> filters_lock (filters_mutex);

        const int id = static_cast<int> (filters.size ());
        filters.push_back (create_bandstop_filter (
            sampling_rate, center_freq, band_width, order, filter_type, ripple));
        *filter_id = id;
    }
    catch (const BrainFlowException &e)
    {
        result = e.exit_code;
    }
    catch (...)
    {
        result = static_cast<int> (BrainFlowExitCodes::GENERAL_ERROR);
    }
    return result;
}

int create_remove_environmental_noise (int *filter_id, int sampling_rate, int noise_type)
{
    int result = static_cast<int> (BrainFlowExitCodes::STATUS_OK);
    try
    {
        const std::lock_guard<std::mutex> filters_lock (filters_mutex);

        const int id = static_cast<int> (filters.size ());
        filters.push_back (create_environmental_noise_filter (sampling_rate, noise_type));
        *filter_id = id;
    }
    catch (const BrainFlowException &e)
    {
        result = e.exit_code;
    }
    catch (...)
    {
        result = static_cast<int> (BrainFlowExitCodes::GENERAL_ERROR);
    }
    return result;
}

int create_rolling (int *filter_id, int period, int agg_operation)
{
    int result = static_cast<int> (BrainFlowExitCodes::STATUS_OK);
    try
    {
        const std::lock_guard<std::mutex> filters_lock (filters_mutex);

        const int id = static_cast<int> (filters.size ());
        filters.push_back (create_rolling_filter (period, agg_operation));
        *filter_id = id;
    }
    catch (const BrainFlowException &e)
    {
        result = e.exit_code;
    }
    catch (...)
    {
        result = static_cast<int> (BrainFlowExitCodes::GENERAL_ERROR);
    }
    return result;
}

int perform_filtering (int filter_id, double *data, int data_len)
{
    int result = static_cast<int> (BrainFlowExitCodes::STATUS_OK);
    try
    {
        auto &&filter = filters[filter_id];
        if (filter)
        {
            filter->process (data, data_len);
        }
        else
        {
            result = static_cast<int> (BrainFlowExitCodes::GENERAL_ERROR);
        }
    }
    catch (const BrainFlowException &e)
    {
        result = e.exit_code;
    }
    catch (...)
    {
        result = static_cast<int> (BrainFlowExitCodes::GENERAL_ERROR);
    }
    return result;
}
int destroy_filter (int filter_id)
{
    int result = static_cast<int> (BrainFlowExitCodes::STATUS_OK);
    try
    {
        const std::lock_guard<std::mutex> filters_lock (filters_mutex);

        filters[filter_id].reset (nullptr);
    }
    catch (const BrainFlowException &e)
    {
        result = e.exit_code;
    }
    catch (...)
    {
        result = static_cast<int> (BrainFlowExitCodes::GENERAL_ERROR);
    }
    return result;
}

#include <condition_variable>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

#include "band_power_settings.h"
#include "psd_workspace.h"

namespace
{
    const int ok = (int)BrainFlowExitCodes::STATUS_OK;
    const int invalid = (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    const int failure = (int)BrainFlowExitCodes::GENERAL_ERROR;
    const int short_buffer = (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;

    struct Worker
    {
        std::vector<double> data, padded, amplitudes, frequencies;
        PsdWorkspace spectrum;
        Worker (int samples, int guard, int nfft, int fs)
            : amplitudes (nfft / 2 + 1), frequencies (nfft / 2 + 1), spectrum (nfft, fs)
        {
            data.reserve (samples);
            padded.reserve ((size_t)samples + 2 * guard);
            if (spectrum.prepare ((int)WindowOperations::HANNING) != ok)
                throw std::invalid_argument ("invalid window");
        }
    };

    struct Context
    {
        int max_channels, max_samples, nfft, guard, detrend_operation;
        std::vector<double> starts, stops, means, deviations;
        std::vector<band_power_helpers::Section> sections;
        std::vector<std::vector<double>> bands;
        std::vector<int> errors;
        std::vector<std::unique_ptr<Worker>> workspaces;
        std::vector<std::thread> threads;
        std::mutex process_mutex, pool_mutex;
        std::condition_variable ready, done;
        bool stopping = false;
        uint64_t generation = 0;
        int next = 0, remaining = 0, channel_count = 0;
        const double *input = nullptr;
        const int *channels = nullptr;
        int samples = 0, stride = 0;

        ~Context ()
        {
            stop ();
        }
        void stop ()
        {
            {
                std::lock_guard<std::mutex> lock (pool_mutex);
                stopping = true;
            }
            ready.notify_all ();
            for (auto &thread : threads)
                if (thread.joinable ())
                    thread.join ();
        }
        void start (int count, int fs)
        {
            for (int i = 0; i < count; i++)
                workspaces.emplace_back (new Worker (max_samples, guard, nfft, fs));
            if (count == 1)
                return;
            // The owning shared_ptr destroys and joins any threads if construction fails.
            for (int i = 0; i < count; i++)
                threads.emplace_back (
                    [this, i]
                    {
                        uint64_t observed = 0;
                        std::unique_lock<std::mutex> lock (pool_mutex);
                        while (true)
                        {
                            ready.wait (lock, [&] { return stopping || generation != observed; });
                            if (stopping)
                                return;
                            observed = generation;
                            while (next < channel_count)
                            {
                                const int channel = next++;
                                lock.unlock ();
                                process_channel (channel, *workspaces[i]);
                                lock.lock ();
                                if (--remaining == 0)
                                    done.notify_one ();
                            }
                        }
                    });
        }
        void process_channel (int index, Worker &worker)
        {
            try
            {
                const double *source = input + (size_t)channels[index] * stride;
                if (!finite_signal (source, samples))
                {
                    errors[index] = invalid;
                    return;
                }
                worker.data.assign (source, source + samples);
                int res = detrend (worker.data.data (), samples, detrend_operation);
                if (res == ok && !sections.empty () &&
                    !band_power_helpers::filter_with_padding (
                        worker.data, sections, guard, worker.padded))
                    res = invalid;
                if (res == ok)
                    res = worker.spectrum.process (worker.data.data () + guard, samples - 2 * guard,
                        (int)(4LL * nfft / 5), worker.amplitudes.data (),
                        worker.frequencies.data ());
                for (size_t band = 0; res == ok && band < starts.size (); band++)
                    res = get_band_power (worker.amplitudes.data (), worker.frequencies.data (),
                        nfft / 2 + 1, starts[band], stops[band], &bands[band][index]);
                errors[index] = res;
            }
            catch (...)
            {
                errors[index] = failure;
            }
        }
        int process (const double *data, int count, int length, int row_stride, const int *selected,
            double *averages, double *variations)
        {
            std::lock_guard<std::mutex> call_lock (process_mutex);
            // Publish the job while holding the same mutex used by the workers.
            std::unique_lock<std::mutex> pool_lock (pool_mutex);
            input = data;
            channels = selected;
            samples = length;
            stride = row_stride;
            channel_count = count;
            if (threads.empty () || count < 8)
            {
                for (int i = 0; i < count; i++)
                    process_channel (i, *workspaces[0]);
            }
            else
            {
                next = 0;
                remaining = count;
                generation++;
                ready.notify_all ();
                done.wait (pool_lock, [&] { return remaining == 0; });
            }
            for (int i = 0; i < count; i++)
                if (errors[i] != ok)
                    return errors[i];
            double scale = 0.0;
            for (const auto &band : bands)
                for (int i = 0; i < count; i++)
                    scale = std::max (scale, band[i]);
            std::fill (means.begin (), means.end (), 0.0);
            std::fill (deviations.begin (), deviations.end (), 0.0);
            if (scale > 0.0)
            {
                double total = 0.0;
                for (size_t band = 0; band < bands.size (); band++)
                {
                    for (int i = 0; i < count; i++)
                        means[band] += (bands[band][i] / scale) / count;
                    for (int i = 0; i < count; i++)
                    {
                        const double difference = bands[band][i] / scale - means[band];
                        deviations[band] += difference * difference / count;
                    }
                    total += means[band];
                }
                for (size_t band = 0; band < bands.size (); band++)
                {
                    deviations[band] =
                        means[band] > 0.0 ? std::sqrt (deviations[band]) / means[band] : 0.0;
                    means[band] /= total;
                }
            }
            std::copy (means.begin (), means.end (), averages);
            std::copy (deviations.begin (), deviations.end (), variations);
            return ok;
        }
    };
    std::mutex registry_mutex;
    std::map<int, std::shared_ptr<Context>> contexts;
    int64_t next_handle = 1;
}

int create_band_power_context (int max_channels, int max_samples, const double *starts,
    const double *stops, int num_bands, int fs, int apply, int nfft, double low, double high,
    int mains, int detrend_operation, int workers, int *handle)
try
{
    if (!handle || !starts || !stops || max_channels < 1 || max_samples < 1 || num_bands < 1 ||
        (int64_t)max_channels * max_samples > std::numeric_limits<int>::max () ||
        (int64_t)max_channels * num_bands > std::numeric_limits<int>::max () ||
        detrend_operation < 0 || detrend_operation > 2 || workers < 0 || workers > 64)
        return invalid;
    auto context = std::make_shared<Context> ();
    for (int i = 0; i < num_bands; i++)
        if (!std::isfinite (starts[i]) || !std::isfinite (stops[i]) || starts[i] < 0 ||
            starts[i] >= stops[i] || stops[i] > fs / 2.0)
            return invalid;
    if (!band_power_settings::prepare_band_power (
            fs, apply, nfft, low, high, mains, context->sections, context->guard))
        return invalid;
    if ((int64_t)nfft + 2LL * context->guard > max_samples)
        return short_buffer;
    context->max_channels = max_channels;
    context->max_samples = max_samples;
    context->nfft = nfft;
    context->detrend_operation = detrend_operation;
    context->starts.assign (starts, starts + num_bands);
    context->stops.assign (stops, stops + num_bands);
    context->means.resize (num_bands);
    context->deviations.resize (num_bands);
    context->errors.resize (max_channels);
    context->bands.assign (num_bands, std::vector<double> (max_channels));
    if (workers == 0)
        workers = (int)std::max (1u, std::min (4u, std::thread::hardware_concurrency ()));
    context->start (std::min (workers, max_channels), fs);
    std::lock_guard<std::mutex> lock (registry_mutex);
    if (next_handle > std::numeric_limits<int>::max ())
        return failure;
    const int id = (int)next_handle++;
    contexts.emplace (id, context);
    *handle = id;
    return ok;
}
catch (...)
{
    return failure;
}

int process_band_power_context (int handle, const double *data, int capacity, int rows, int samples,
    int stride, const int *channels, int count, double *averages, double *deviations,
    int output_capacity)
try
{
    std::shared_ptr<Context> context;
    {
        std::lock_guard<std::mutex> lock (registry_mutex);
        auto found = contexts.find (handle);
        if (found == contexts.end ())
            return invalid;
        context = found->second;
    }
    if (!data || !channels || !averages || !deviations || rows < 1 || count < 1 || samples < 1 ||
        stride < samples || count > context->max_channels || samples > context->max_samples ||
        capacity < 0 || (int64_t)(rows - 1) * stride + samples > capacity)
        return invalid;
    if (output_capacity < (int)context->starts.size () ||
        samples < (int64_t)context->nfft + 2LL * context->guard)
        return short_buffer;
    for (int i = 0; i < count; i++)
        if (channels[i] < 0 || channels[i] >= rows)
            return invalid;
    return context->process (data, count, samples, stride, channels, averages, deviations);
}
catch (...)
{
    return failure;
}

int release_band_power_context (int handle)
try
{
    std::shared_ptr<Context> context;
    {
        std::lock_guard<std::mutex> lock (registry_mutex);
        auto found = contexts.find (handle);
        if (found == contexts.end ())
            return invalid;
        context = found->second;
        contexts.erase (found);
    }
    // Joining workers must not hold the registry lock for unrelated contexts.
    return ok;
}
catch (...)
{
    return failure;
}

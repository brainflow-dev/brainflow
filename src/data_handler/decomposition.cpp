#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

#include "data_handler.h"
#include "fastica.h"
#include "spdlog/spdlog.h"
#include "wauxlib.h"
#include "wavelet_helpers.h"
#include "wavelib.h"

extern std::shared_ptr<spdlog::logger> data_logger;

namespace
{
    typedef std::unique_ptr<wave_set, decltype (&wave_free)> Wave;
    typedef std::unique_ptr<wt_set, decltype (&wt_free)> Transform;
    typedef std::unique_ptr<denoise_set, decltype (&denoise_free)> Denoiser;
    typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> RowMatrix;

    bool finite_samples (const double *data, size_t count)
    {
        if (!data)
        {
            return false;
        }
        for (size_t i = 0; i < count; i++)
        {
            if (!std::isfinite (data[i]))
            {
                return false;
            }
        }
        return true;
    }

    bool valid_wavelet_configuration (int count, int level, wave_object wave)
    {
        if (count <= 0 || level <= 0 || level > 100)
        {
            return false;
        }
        // Wavelib uses int offsets and allocates count + 2*level*(filter_length+1).
        long long capacity = (long long)count + 2LL * level * (wave->filtlength + 1);
        if (capacity > std::numeric_limits<int>::max ())
        {
            return false;
        }
        double maximum = std::log2 ((double)count / (wave->filtlength - 1));
        return level <= maximum;
    }

    int inverse_wavelet (double *coefficients, int coefficient_count, int original_count,
        int wavelet, int level, int extension, int *lengths, int lengths_count, double *output,
        int output_count, bool checked)
    {
        std::string wavelet_name = get_wavelet_name (wavelet);
        std::string extension_name = get_extension_type (extension);
        if (!coefficients || !lengths || !output || original_count <= 0 || level <= 0 ||
            level > 100 || wavelet_name.empty () || extension_name.empty () ||
            (checked &&
                (lengths_count != level + 1 || output_count < original_count ||
                    coefficient_count < original_count)))
        {
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
        try
        {
            Wave wave (wave_init (wavelet_name.c_str ()), wave_free);
            if (!valid_wavelet_configuration (original_count, level, wave.get ()))
            {
                return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
            }
            // Derive metadata from the configuration, never from caller-provided sizes.
            std::vector<int> expected (level + 1);
            int count = original_count;
            int total = 0;
            for (int i = level; i > 0; i--)
            {
                count = (int)(((long long)count +
                                  (extension == (int)WaveletExtensionTypes::PERIODIC ?
                                          1 :
                                          wave->filtlength - 1)) /
                    2);
                expected[i] = count;
                total += count;
            }
            expected[0] = expected[1];
            total += expected[0];
            for (int i = 0; i <= level; i++)
            {
                if (lengths[i] != expected[i])
                {
                    return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
                }
            }
            if (checked && coefficient_count != total)
            {
                return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
            }
            if (!finite_samples (coefficients, total))
            {
                return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
            }
            Transform transform (wt_init (wave.get (), "dwt", original_count, level), wt_free);
            setDWTExtension (transform.get (), extension_name.c_str ());
            setWTConv (transform.get (), "direct");
            std::copy (expected.begin (), expected.end (), transform->length);
            transform->length[level + 1] = original_count;
            transform->outlength = total;
            transform->zpad = 0;
            std::copy (coefficients, coefficients + total, transform->output);
            std::vector<double> restored (original_count);
            idwt (transform.get (), restored.data ());
            if (!finite_samples (restored.data (), restored.size ()))
            {
                return (int)BrainFlowExitCodes::GENERAL_ERROR;
            }
            std::copy (restored.begin (), restored.end (), output);
            return (int)BrainFlowExitCodes::STATUS_OK;
        }
        catch (const std::exception &error)
        {
            data_logger->error ("Inverse wavelet failed: {}", error.what ());
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }
}

int perform_wavelet_transform (double *data, int data_len, int wavelet, int decomposition_level,
    int extension, double *output_data, int *decomposition_lengths)
{
    std::string wavelet_name = get_wavelet_name (wavelet);
    std::string extension_name = get_extension_type (extension);
    if (data_len <= 0 || decomposition_level <= 0 || decomposition_level > 100 ||
        wavelet_name.empty () || extension_name.empty () || !output_data ||
        !decomposition_lengths || !finite_samples (data, data_len))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    try
    {
        Wave wave (wave_init (wavelet_name.c_str ()), wave_free);
        if (!valid_wavelet_configuration (data_len, decomposition_level, wave.get ()))
        {
            return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
        }
        Transform transform (wt_init (wave.get (), "dwt", data_len, decomposition_level), wt_free);
        setDWTExtension (transform.get (), extension_name.c_str ());
        setWTConv (transform.get (), "direct");
        dwt (transform.get (), data);
        if (!finite_samples (transform->output, transform->outlength))
        {
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        std::copy (transform->output, transform->output + transform->outlength, output_data);
        std::copy (
            transform->length, transform->length + decomposition_level + 1, decomposition_lengths);
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    catch (const std::exception &error)
    {
        data_logger->error ("Wavelet transform failed: {}", error.what ());
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
}

int perform_inverse_wavelet_transform (double *coefficients, int original_count, int wavelet,
    int level, int extension, int *lengths, double *output)
{
    // Legacy raw-pointer ABI cannot know actual allocation sizes. All bindings should
    // use the checked entry point; this entry still rejects inconsistent metadata.
    return inverse_wavelet (
        coefficients, 0, original_count, wavelet, level, extension, lengths, 0, output, 0, false);
}

int perform_inverse_wavelet_transform_checked (double *coefficients, int coefficient_count,
    int original_count, int wavelet, int level, int extension, int *lengths, int lengths_count,
    double *output, int output_count)
{
    return inverse_wavelet (coefficients, coefficient_count, original_count, wavelet, level,
        extension, lengths, lengths_count, output, output_count, true);
}

int perform_wavelet_denoising (double *data, int data_len, int wavelet, int decomposition_level,
    int wavelet_denoising, int threshold, int extension, int noise_level)
{
    std::string wavelet_name = get_wavelet_name (wavelet);
    std::string denoising_name = get_wavelet_denoising_type (wavelet_denoising);
    std::string threshold_name = get_threshold_type (threshold);
    std::string extension_name = get_extension_type (extension);
    std::string noise_name = get_noise_estimation_type (noise_level);
    if (data_len <= 0 || decomposition_level <= 0 || decomposition_level > 100 ||
        wavelet_name.empty () || denoising_name.empty () || threshold_name.empty () ||
        extension_name.empty () || noise_name.empty () || !finite_samples (data, data_len))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    try
    {
        Wave wave (wave_init (wavelet_name.c_str ()), wave_free);
        if (!valid_wavelet_configuration (data_len, decomposition_level, wave.get ()))
        {
            return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
        }
        Denoiser denoiser (
            denoise_init (data_len, decomposition_level, wavelet_name.c_str ()), denoise_free);
        setDenoiseMethod (denoiser.get (), denoising_name.c_str ());
        setDenoiseWTMethod (denoiser.get (), "dwt");
        setDenoiseWTExtension (denoiser.get (), extension_name.c_str ());
        setDenoiseParameters (denoiser.get (), threshold_name.c_str (), noise_name.c_str ());
        std::vector<double> result (data_len);
        denoise (denoiser.get (), data, result.data ());
        if (!finite_samples (result.data (), result.size ()))
        {
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        std::copy (result.begin (), result.end (), data);
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    catch (const std::exception &error)
    {
        data_logger->error ("Wavelet denoising failed: {}", error.what ());
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
}

int restore_data_from_wavelet_detailed_coeffs (double *data, int data_len, int wavelet,
    int decomposition_level, int level_to_restore, double *output)
{
    std::string wavelet_name = get_wavelet_name (wavelet);
    if (data_len <= 0 || decomposition_level <= 0 || decomposition_level > 100 ||
        level_to_restore <= 0 || level_to_restore > decomposition_level || wavelet_name.empty () ||
        !output || !finite_samples (data, data_len))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    try
    {
        Wave wave (wave_init (wavelet_name.c_str ()), wave_free);
        if (!valid_wavelet_configuration (data_len, decomposition_level, wave.get ()))
        {
            return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
        }
        Transform transform (wt_init (wave.get (), "dwt", data_len, decomposition_level), wt_free);
        setDWTExtension (transform.get (), "sym");
        setWTConv (transform.get (), "direct");
        dwt (transform.get (), data);
        int offset = 0;
        for (int i = 0; i <= decomposition_level; i++)
        {
            if (i == 0 || decomposition_level + 1 - i != level_to_restore)
            {
                std::fill (transform->output + offset,
                    transform->output + offset + transform->length[i], 0.0);
            }
            offset += transform->length[i];
        }
        std::vector<double> restored (data_len);
        idwt (transform.get (), restored.data ());
        if (!finite_samples (restored.data (), restored.size ()))
        {
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        std::copy (restored.begin (), restored.end (), output);
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    catch (const std::exception &error)
    {
        data_logger->error ("Wavelet detail reconstruction failed: {}", error.what ());
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
}

int get_csp (const double *data, const double *labels, int n_epochs, int n_channels, int n_times,
    double *output_w, double *output_d)
{
    if (!data || !labels || !output_w || !output_d || n_epochs < 2 || n_channels <= 0 ||
        n_times < 2 || (long long)n_channels * n_times > std::numeric_limits<int>::max () ||
        (long long)n_channels * n_channels > std::numeric_limits<int>::max () ||
        (long long)n_epochs * n_channels * n_times > std::numeric_limits<int>::max ())
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    int class_counts[2] = {0, 0};
    for (int epoch = 0; epoch < n_epochs; epoch++)
    {
        if (labels[epoch] != 0.0 && labels[epoch] != 1.0)
        {
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
        class_counts[(int)labels[epoch]]++;
    }
    size_t total = (size_t)n_epochs * n_channels * n_times;
    if (!class_counts[0] || !class_counts[1] || !finite_samples (data, total))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    double scale = 0.0;
    for (size_t i = 0; i < total; i++)
    {
        scale = std::max (scale, std::abs (data[i]));
    }
    if (!(scale > 0.0))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    try
    {
        Eigen::MatrixXd covariances[2] = {Eigen::MatrixXd::Zero (n_channels, n_channels),
            Eigen::MatrixXd::Zero (n_channels, n_channels)};
        for (int epoch = 0; epoch < n_epochs; epoch++)
        {
            Eigen::MatrixXd centered =
                Eigen::Map<const RowMatrix> (
                    data + (size_t)epoch * n_channels * n_times, n_channels, n_times) /
                scale;
            Eigen::VectorXd means = centered.rowwise ().mean ();
            centered.colwise () -= means;
            covariances[(int)labels[epoch]] += centered * (centered / (double)n_times).transpose ();
        }
        covariances[0] /= (double)class_counts[0];
        covariances[1] /= (double)class_counts[1];
        Eigen::MatrixXd composite = covariances[0] + covariances[1];
        Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> rank_solver (
            composite, Eigen::EigenvaluesOnly);
        if (rank_solver.info () != Eigen::Success || !rank_solver.eigenvalues ().allFinite ())
        {
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        double largest = rank_solver.eigenvalues ()[n_channels - 1];
        if (!(largest > 0.0) ||
            rank_solver.eigenvalues ()[0] <=
                largest * std::numeric_limits<double>::epsilon () * std::max (n_channels, n_times))
        {
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
        Eigen::GeneralizedSelfAdjointEigenSolver<Eigen::MatrixXd> solver (
            covariances[0], composite);
        if (solver.info () != Eigen::Success || !solver.eigenvalues ().allFinite () ||
            !solver.eigenvectors ().allFinite ())
        {
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        Eigen::MatrixXd filters = solver.eigenvectors ().transpose () / scale;
        if (!filters.allFinite ())
        {
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        Eigen::Map<RowMatrix> output_filters (output_w, n_channels, n_channels);
        Eigen::Map<Eigen::VectorXd> output_values (output_d, n_channels);
        output_filters = filters;
        output_values = solver.eigenvalues ();
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    catch (const std::exception &error)
    {
        data_logger->error ("CSP failed: {}", error.what ());
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
}

int perform_ica_with_options (double *data, int rows, int cols, int num_components, double *w_mat,
    double *k_mat, double *a_mat, double *s_mat, int max_iterations, double tolerance, int seed)
{
    if (!data || !w_mat || !k_mat || !a_mat || !s_mat || rows < 2 || cols < 3 ||
        num_components < 2 || num_components > std::min (rows, cols - 1) ||
        (long long)rows * cols > std::numeric_limits<int>::max () || max_iterations <= 0 ||
        !std::isfinite (tolerance) || tolerance <= 0.0 || tolerance >= 1.0 || seed < -1)
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    try
    {
        Eigen::MatrixXd input = Eigen::Map<const RowMatrix> (data, rows, cols);
        FastICA ica (num_components, max_iterations, tolerance, seed);
        int result = ica.compute (input);
        if (result == (int)BrainFlowExitCodes::STATUS_OK)
        {
            result = ica.get_matrixes (w_mat, k_mat, a_mat, s_mat);
        }
        else if (result == (int)BrainFlowExitCodes::GENERAL_ERROR)
        {
            data_logger->error (
                "ICA failed to converge to a finite solution in {} iterations", max_iterations);
        }
        return result;
    }
    catch (const std::exception &error)
    {
        data_logger->error ("ICA failed: {}", error.what ());
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
}

int perform_ica (double *data, int rows, int cols, int num_components, double *w_mat, double *k_mat,
    double *a_mat, double *s_mat)
{
    return perform_ica_with_options (
        data, rows, cols, num_components, w_mat, k_mat, a_mat, s_mat, 1000, 0.0001, -1);
}

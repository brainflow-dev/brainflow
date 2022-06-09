#include <iostream>
#include <stdlib.h>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "board_shim.h"
#include "data_filter.h"
#include "ml_model.h"

using namespace std;

bool parse_args (int argc, char *argv[], struct BrainFlowInputParams *params, int *board_id,
    struct BrainFlowModelParams *model_params);


int main (int argc, char *argv[])
{
    BoardShim::enable_dev_board_logger ();
    MLModel::enable_dev_ml_logger ();

    struct BrainFlowInputParams params;
    struct BrainFlowModelParams model_params (
        (int)BrainFlowMetrics::RESTFULNESS, (int)BrainFlowClassifiers::DEFAULT_CLASSIFIER);
    int board_id = 0;
    if (!parse_args (argc, argv, &params, &board_id, &model_params))
    {
        return -1;
    }
    int res = 0;

    BoardShim *board = new BoardShim (board_id, params);
    int master_board_id = 0;

    try
    {
        board->prepare_session ();
        master_board_id = board->get_board_id ();
        board->start_stream ();

#ifdef _WIN32
        Sleep (5000);
#else
        sleep (5);
#endif

        board->stop_stream ();
        BrainFlowArray<double, 2> data = board->get_board_data ();
        board->release_session ();
        std::cout << data << std::endl;
        // calc band powers
        int sampling_rate = BoardShim::get_sampling_rate (master_board_id);
        std::vector<int> eeg_channels = BoardShim::get_eeg_channels (master_board_id);
        std::pair<double *, double *> bands =
            DataFilter::get_avg_band_powers (data, eeg_channels, sampling_rate, true);

        MLModel model (model_params);
        model.prepare ();
        std::cout << "Score :";
        for (double score : model.predict (bands.first, 5))
        {
            std::cout << score << std::endl;
        }
        model.release ();

        delete[] bands.first;
        delete[] bands.second;
    }
    catch (const BrainFlowException &err)
    {
        BoardShim::log_message ((int)LogLevels::LEVEL_ERROR, err.what ());
        res = err.exit_code;
        if (board->is_prepared ())
        {
            board->release_session ();
        }
    }

    delete board;

    return res;
}

bool parse_args (int argc, char *argv[], struct BrainFlowInputParams *params, int *board_id,
    struct BrainFlowModelParams *model_params)
{
    bool board_id_found = false;
    bool classifier_found = false;
    bool metric_found = false;
    for (int i = 1; i < argc; i++)
    {
        if (std::string (argv[i]) == std::string ("--board-id"))
        {
            if (i + 1 < argc)
            {
                i++;
                board_id_found = true;
                *board_id = std::stoi (std::string (argv[i]));
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string (argv[i]) == std::string ("--classifier"))
        {
            if (i + 1 < argc)
            {
                i++;
                classifier_found = true;
                model_params->classifier = std::stoi (std::string (argv[i]));
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string (argv[i]) == std::string ("--metric"))
        {
            if (i + 1 < argc)
            {
                i++;
                metric_found = true;
                model_params->metric = std::stoi (std::string (argv[i]));
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string (argv[i]) == std::string ("--model-file"))
        {
            if (i + 1 < argc)
            {
                i++;
                model_params->file = std::string (argv[i]);
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string (argv[i]) == std::string ("--output-name"))
        {
            if (i + 1 < argc)
            {
                i++;
                model_params->output_name = std::string (argv[i]);
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string (argv[i]) == std::string ("--metric"))
        {
            if (i + 1 < argc)
            {
                i++;
                metric_found = true;
                model_params->metric = std::stoi (std::string (argv[i]));
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string (argv[i]) == std::string ("--ip-address"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->ip_address = std::string (argv[i]);
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string (argv[i]) == std::string ("--ip-port"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->ip_port = std::stoi (std::string (argv[i]));
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string (argv[i]) == std::string ("--serial-port"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->serial_port = std::string (argv[i]);
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string (argv[i]) == std::string ("--ip-protocol"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->ip_protocol = std::stoi (std::string (argv[i]));
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string (argv[i]) == std::string ("--timeout"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->timeout = std::stoi (std::string (argv[i]));
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string (argv[i]) == std::string ("--other-info"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->other_info = std::string (argv[i]);
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string (argv[i]) == std::string ("--mac-address"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->mac_address = std::string (argv[i]);
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string (argv[i]) == std::string ("--serial-number"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->serial_number = std::string (argv[i]);
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string (argv[i]) == std::string ("--file"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->file = std::string (argv[i]);
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
    }
    if (!board_id_found)
    {
        std::cerr << "board id is not provided" << std::endl;
        return false;
    }
    if (!classifier_found)
    {
        std::cerr << "classifier is not provided" << std::endl;
        return false;
    }
    if (!metric_found)
    {
        std::cerr << "metric is not provided" << std::endl;
        return false;
    }
    return true;
}

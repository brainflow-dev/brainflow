#include <chrono>
#include <iostream>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "board_shim.h"
#include "data_filter.h"
#include "ml_model.h"

using namespace std;
using namespace std::chrono;


bool parse_args (int argc, char *argv[], struct BrainFlowInputParams *params, int *board_id);

int main (int argc, char *argv[])
{
    struct BrainFlowInputParams params;
    int board_id = 0;
    if (!parse_args (argc, argv, &params, &board_id))
    {
        return -1;
    }

    BoardShim::enable_dev_board_logger ();

    BoardShim *board = new BoardShim (board_id, params);
    double **data = NULL;
    int *eeg_channels = NULL;
    int num_rows = 0;
    int res = 0;
    int master_board_id = 0;

    if ((board_id == (int)BoardIds::STREAMING_BOARD) ||
        (board_id == (int)BoardIds::PLAYBACK_FILE_BOARD))
    {
        try
        {
            master_board_id = std::stoi (params.other_info);
        }
        catch (const std::exception &e)
        {
            throw BrainFlowException ("specify master board id using params.other_info",
                (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
        }
    }

    int sampling_rate = BoardShim::get_sampling_rate (master_board_id);

    try
    {
        // Collect data from device
        board->prepare_session ();
        board->start_stream ();
        BoardShim::log_message ((int)LogLevels::LEVEL_INFO, "Start sleeping in the main thread");
        // recommended window size for eeg metric calculation is at least 4 seconds, bigger is
        // better
#ifdef _WIN32
        Sleep (5000);
#else
        sleep (5);
#endif
        int data_count = 0;
        data = board->get_board_data (&data_count);
        board->stop_stream ();
        std::cout << "Data Count: " << data_count << std::endl;
        board->release_session ();
        num_rows = BoardShim::get_num_rows (master_board_id);

        // Calc bandpowers and build feature vector
        int eeg_num_channels = 0;
        eeg_channels = BoardShim::get_eeg_channels (master_board_id, &eeg_num_channels);
        std::pair<double *, double *> bands = DataFilter::get_avg_band_powers (
            data, data_count, eeg_channels, eeg_num_channels, sampling_rate, true);
        double feature_vector[10];
        for (int i = 0; i < 5; i++)
        {
            feature_vector[i] = bands.first[i];
            feature_vector[i + 5] = bands.second[i];
        }
        for (int i = 0; i < 10; i++)
        {
            std::cout << feature_vector[i] << " ";
        }
        std::cout << std::endl;

        // Testing all classifiers and metric types
        struct BrainFlowModelParams conc_model_params (0, 0);
        MLModel concentration_model (conc_model_params);
        string metric_type = "Concentration";
        for (int metric = 1; metric >= 0; metric--)
        {
            if (!metric)
                metric_type = "Relaxation";
            for (int classifier = (int)BrainFlowClassifiers::REGRESSION;
                 classifier <= (int)BrainFlowClassifiers::LDA; classifier++)
            {
                conc_model_params = BrainFlowModelParams (metric, classifier);
                concentration_model = MLModel (conc_model_params);
                concentration_model.prepare ();
                std::cout << metric_type << " Classifier " << classifier << ":"
                          << concentration_model.predict (feature_vector, 10) << std::endl;
                concentration_model.release ();
            }
        }

        delete[] bands.first;
        delete[] bands.second;
    }
    catch (const BrainFlowException &err)
    {
        BoardShim::log_message ((int)LogLevels::LEVEL_ERROR, err.what ());
        res = err.exit_code;
    }

    if (data != NULL)
    {
        for (int i = 0; i < num_rows; i++)
        {
            delete[] data[i];
        }
    }
    delete[] data;
    delete[] eeg_channels;
    delete board;

    return res;
}


bool parse_args (int argc, char *argv[], struct BrainFlowInputParams *params, int *board_id)
{
    bool board_id_found = false;
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
    return true;
}
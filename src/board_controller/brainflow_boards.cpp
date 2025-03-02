#include "brainflow_boards.h"

// clang-format off

BrainFlowBoards::BrainFlowBoards()
{

    /* For all real boards there are four required fields:
     *   name
     *   num_rows
     *   timestamp_channel
     *   marker_channel
     * Nice to set:
     *   package_num
     *   sampling_rate
     * Everything else is optional and up to device
    */

    brainflow_boards_json = {
        {"boards", {
            {"-3", json::object()},
            {"-2", json::object()},
            {"-1", json::object()},
            {"0", json::object()},
            {"1", json::object()},
            {"2", json::object()},
            {"3", json::object()},
            {"4", json::object()},
            {"5", json::object()},
            {"6", json::object()},
            {"7", json::object()},
            {"8", json::object()},
            {"9", json::object()},
            {"10", json::object()},
            {"11", json::object()},
            {"13", json::object()},
            {"14", json::object()},
            {"16", json::object()},
            {"17", json::object()},
            {"18", json::object()},
            {"19", json::object()},
            {"20", json::object()},
            {"21", json::object()},
            {"22", json::object()},
            {"23", json::object()},
            {"24", json::object()},
            {"25", json::object()},
            {"26", json::object()},
            {"27", json::object()},
            {"28", json::object()},
            {"29", json::object()},
            {"30", json::object()},
            {"31", json::object()},
            {"32", json::object()},
            {"33", json::object()},
            {"34", json::object()},
            {"35", json::object()},
            {"36", json::object()},
            {"37", json::object()},
            {"38", json::object()},
            {"39", json::object()},
            {"40", json::object()},
            {"41", json::object()},
            {"42", json::object()},
            {"44", json::object()},
            {"45", json::object()},
            {"46", json::object()},
            {"47", json::object()},
            {"48", json::object()},
            {"49", json::object()},
            {"50", json::object()},
            {"51", json::object()},
            {"52", json::object()},
            {"53", json::object()},
            {"54", json::object()},
            {"55", json::object()},
            {"56", json::object()},
            {"57", json::object()},
            {"58", json::object()},
            {"59", json::object()},
            {"60", json::object()},
            {"61", json::object()},
            {"62", json::object()},
            {"63", json::object()},
            {"64", json::object()}
        }
    }};

    // available presets are: default, auxiliary, ancillary, default is required, other presets are optional
    brainflow_boards_json["boards"]["-3"]["default"] = {{"name", "PlayBack"}};
    brainflow_boards_json["boards"]["-3"]["auxiliary"] = {{"name", "PlayBack"}};
    brainflow_boards_json["boards"]["-3"]["ancillary"] = {{"name", "PlayBack"}};
    brainflow_boards_json["boards"]["-2"]["default"] = {{"name", "Streaming"}};
    brainflow_boards_json["boards"]["-2"]["auxiliary"] = {{"name", "Streaming"}};
    brainflow_boards_json["boards"]["-2"]["ancillary"] = {{"name", "Streaming"}};
    brainflow_boards_json["boards"]["-1"]["default"] =
    {
        {"name", "Synthetic"},
        {"sampling_rate", 250},
        {"package_num_channel", 0},
        {"battery_channel", 29},
        {"timestamp_channel", 30},
        {"marker_channel", 31},
        {"num_rows", 32},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
        {"eeg_names", "Fz,C3,Cz,C4,Pz,PO7,Oz,PO8,F5,F7,F3,F1,F2,F4,F6,F8"},
        {"emg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
        {"ecg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
        {"eog_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
        {"accel_channels", {17, 18, 19}},
        {"gyro_channels", {20, 21, 22}},
        {"eda_channels", {23}},
        {"ppg_channels", {24, 25}},
        {"temperature_channels", {26}},
        {"resistance_channels", {27, 28}}
    };
    brainflow_boards_json["boards"]["-1"]["auxiliary"] =
    {
        {"name", "SyntheticAux"},
        {"sampling_rate", 250},
        {"package_num_channel", 0},
        {"timestamp_channel", 18},
        {"marker_channel", 19},
        {"num_rows", 20},
        {"other_channels", {14, 15, 16, 17}},
        {"battery_channel", 1},
        {"accel_channels", {2, 3, 4}},
        {"gyro_channels", {5, 6, 7}},
        {"eda_channels", {8}},
        {"ppg_channels", {9, 10}},
        {"temperature_channels", {11}},
        {"resistance_channels", {12, 13}}
    };
    brainflow_boards_json["boards"]["0"]["default"] =
    {
        {"name", "Cyton"},
        {"sampling_rate", 250},
        {"package_num_channel", 0},
        {"timestamp_channel", 22},
        {"marker_channel", 23},
        {"num_rows", 24},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"eeg_names", "Fp1,Fp2,C3,C4,P7,P8,O1,O2"},
        {"emg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"ecg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"eog_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"accel_channels", {9, 10, 11}},
        {"analog_channels", {19, 20, 21}},
        {"other_channels", {12, 13, 14, 15, 16, 17, 18}}
    };
    brainflow_boards_json["boards"]["1"]["default"] =
    {
        {"name", "Ganglion"},
        {"sampling_rate", 200},
        {"package_num_channel", 0},
        {"timestamp_channel", 13},
        {"marker_channel", 14},
        {"num_rows", 15},
        {"eeg_channels", {1, 2, 3, 4}},
        {"emg_channels", {1, 2, 3, 4}},
        {"ecg_channels", {1, 2, 3, 4}},
        {"eog_channels", {1, 2, 3, 4}},
        {"accel_channels", {5, 6, 7}},
        {"resistance_channels", {8, 9, 10, 11, 12}}
    };
    brainflow_boards_json["boards"]["2"]["default"] =
    {
        {"name", "CytonDaisy"},
        {"sampling_rate", 125},
        {"package_num_channel", 0},
        {"timestamp_channel", 30},
        {"marker_channel", 31},
        {"num_rows", 32},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
        {"eeg_names", "Fp1,Fp2,C3,C4,P7,P8,O1,O2,F7,F8,F3,F4,T7,T8,P3,P4"},
        {"emg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
        {"ecg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
        {"eog_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
        {"accel_channels", {17, 18, 19}},
        {"analog_channels", {27, 28, 29}},
        {"other_channels", {20, 21, 22, 23, 24, 25, 26}}
    };
    brainflow_boards_json["boards"]["3"]["default"] =
    {
        {"name", "Galea"},
        {"sampling_rate", 250},
        {"package_num_channel", 0},
        {"timestamp_channel", 19},
        {"marker_channel", 20},
        {"num_rows", 21},
        {"eeg_channels", {7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
        {"eeg_names", "FP1,FP2,Fz,Cz,Pz,Oz,P3,P4,O1,O2"},
        {"emg_channels", {1, 2, 3, 4}},
        {"eog_channels", {5, 6}},
        {"other_channels", {17, 18}},
    };
    brainflow_boards_json["boards"]["3"]["auxiliary"] =
    {
        {"name", "GaleaAuxiliary"},
        {"sampling_rate", 50},
        {"package_num_channel", 0},
        {"timestamp_channel", 8},
        {"marker_channel", 9},
        {"num_rows", 10},
        {"battery_channel", 5},
        {"eda_channels", {1}},
        {"ppg_channels", {2, 3}},
        {"other_channels", {6, 7}},
        {"temperature_channels", {4}}
    };
    brainflow_boards_json["boards"]["4"]["default"] =
    {
        {"name", "GanglionWifi"},
        {"sampling_rate", 1600},
        {"package_num_channel", 0},
        {"timestamp_channel", 23},
        {"marker_channel", 24},
        {"num_rows", 25},
        {"eeg_channels", {1, 2, 3, 4}},
        {"emg_channels", {1, 2, 3, 4}},
        {"ecg_channels", {1, 2, 3, 4}},
        {"eog_channels", {1, 2, 3, 4}},
        {"accel_channels", {5, 6, 7}},
        {"analog_channels", {15, 16, 17}},
        {"other_channels", {8, 9, 10, 11, 12, 13, 14}},
        {"resistance_channels", {18, 19, 20, 21, 22}}
    };
    brainflow_boards_json["boards"]["5"]["default"] =
    {
        {"name", "CytonWifi"},
        {"sampling_rate", 1000},
        {"package_num_channel", 0},
        {"timestamp_channel", 22},
        {"marker_channel", 23},
        {"num_rows", 24},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"emg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"ecg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"eog_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"accel_channels", {9, 10, 11}},
        {"analog_channels", {19, 20, 21}},
        {"other_channels", {12, 13, 14, 15, 16, 17, 18}}
    };
    brainflow_boards_json["boards"]["6"]["default"] =
    {
        {"name", "CytonDaisyWifi"},
        {"sampling_rate", 1000},
        {"package_num_channel", 0},
        {"timestamp_channel", 30},
        {"marker_channel", 31},
        {"num_rows", 32},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
        {"emg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
        {"ecg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
        {"eog_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
        {"accel_channels", {17, 18, 19}},
        {"analog_channels", {27, 28, 29}},
        {"other_channels", {20, 21, 22, 23, 24, 25, 26}}
    };
    brainflow_boards_json["boards"]["7"]["default"] =
    {
        {"name", "BrainBit"},
        {"sampling_rate", 250},
        {"timestamp_channel", 10},
        {"marker_channel", 11},
        {"package_num_channel", 0},
        {"battery_channel", 9},
        {"num_rows", 12},
        {"eeg_channels", {1, 2, 3, 4}},
        {"eeg_names", "T3,T4,O1,O2"},
        {"resistance_channels", {5, 6, 7, 8}}
    };
    brainflow_boards_json["boards"]["8"]["default"] =
    {
        {"name", "Unicorn"},
        {"sampling_rate", 250},
        {"timestamp_channel", 17},
        {"marker_channel", 18},
        {"package_num_channel", 15},
        {"num_rows", 19},
        {"eeg_channels", {0, 1, 2, 3, 4, 5, 6, 7}},
        {"eeg_names", "Fz,C3,Cz,C4,Pz,PO7,Oz,PO8"},
        {"accel_channels", {8, 9, 10}},
        {"gyro_channels", {11, 12, 13}},
        {"other_channels", {16}},
        {"battery_channel", 14}
    };
    brainflow_boards_json["boards"]["9"]["default"] =
    {
        {"name", "CallibriEEG"},
        {"sampling_rate", 250},
        {"timestamp_channel", 2},
        {"marker_channel", 3},
        {"package_num_channel", 0},
        {"num_rows", 4},
        {"eeg_channels", {1}}
    };
    brainflow_boards_json["boards"]["10"]["default"] =
    {
        {"name", "CallibriEMG"},
        {"sampling_rate", 1000},
        {"timestamp_channel", 2},
        {"marker_channel", 3},
        {"package_num_channel", 0},
        {"num_rows", 4},
        {"emg_channels", {1}}
    };
    brainflow_boards_json["boards"]["11"]["default"] =
    {
        {"name", "CallibriECG"},
        {"sampling_rate", 125},
        {"timestamp_channel", 2},
        {"marker_channel", 3},
        {"package_num_channel", 0},
        {"num_rows", 4},
        {"ecg_channels", {1}}
    };
    brainflow_boards_json["boards"]["13"]["default"] =
    {
        {"name", "NotionOSC1"},
        {"sampling_rate", 250},
        {"timestamp_channel", 10},
        {"marker_channel", 11},
        {"package_num_channel", 0},
        {"num_rows", 12},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"eeg_names", "CP6,F6,C4,CP4,CP3,F5,C3,CP5"},
        {"other_channels", {9}}
    };
    brainflow_boards_json["boards"]["14"]["default"] =
    {
        {"name", "NotionOSC2"},
        {"sampling_rate", 250},
        {"timestamp_channel", 10},
        {"marker_channel", 11},
        {"package_num_channel", 0},
        {"num_rows", 12},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"eeg_names", "CP5,F5,C3,CP3,CP6,F6,C4,CP4"},
        {"other_channels", {9}}
    };
    brainflow_boards_json["boards"]["16"]["default"] =
    {
        {"name", "GforcePro"},
        {"sampling_rate", 500},
        {"package_num_channel", 0},
        {"timestamp_channel", 9},
        {"marker_channel", 10},
        {"num_rows", 11},
        {"emg_channels", {1, 2, 3, 4, 5, 6, 7, 8}}
    };
    brainflow_boards_json["boards"]["17"]["default"] =
    {
        {"name", "FreeEEG32"},
        {"sampling_rate", 512},
        {"timestamp_channel", 33},
        {"marker_channel", 34},
        {"package_num_channel", 0},
        {"num_rows", 35},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32}},
        {"emg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32}},
        {"ecg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32}}
    };
    brainflow_boards_json["boards"]["18"]["default"] =
    {
        {"name", "BrainBitBLED"},
        {"sampling_rate", 250},
        {"battery_channel", 5},
        {"timestamp_channel", 6},
        {"marker_channel", 7},
        {"package_num_channel", 0},
        {"num_rows", 8},
        {"eeg_channels", {1, 2, 3, 4}},
        {"eeg_names", "T3,T4,O1,O2"}
    };
    brainflow_boards_json["boards"]["19"]["default"] =
    {
        {"name", "GforceDual"},
        {"sampling_rate", 500},
        {"timestamp_channel", 3},
        {"marker_channel", 4},
        {"package_num_channel", 0},
        {"num_rows", 5},
        {"emg_channels", {1, 2}}
    };
    brainflow_boards_json["boards"]["20"]["default"] =
    {
        {"name", "GaleaSerial"},
        {"sampling_rate", 250},
        {"package_num_channel", 0},
        {"timestamp_channel", 19},
        {"marker_channel", 20},
        {"num_rows", 21},
        {"eeg_channels", {7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
        {"eeg_names", "FP1,FP2,Fz,Cz,Pz,Oz,P3,P4,O1,O2"},
        {"emg_channels", {1, 2, 3, 4}},
        {"eog_channels", {5, 6}},
        {"other_channels", {17, 18}},
    };
    brainflow_boards_json["boards"]["20"]["auxiliary"] =
    {
        {"name", "GaleaSerialAuxiliary"},
        {"sampling_rate", 50},
        {"package_num_channel", 0},
        {"timestamp_channel", 8},
        {"marker_channel", 9},
        {"num_rows", 10},
        {"battery_channel", 5},
        {"eda_channels", {1}},
        {"ppg_channels", {2, 3}},
        {"other_channels", {6, 7}},
        {"temperature_channels", {4}}
    };
    brainflow_boards_json["boards"]["21"]["default"] =
    {
        {"name", "MuseSBLED"},
        {"sampling_rate", 256},
        {"timestamp_channel", 6},
        {"marker_channel", 7},
        {"package_num_channel", 0},
        {"num_rows", 8},
        {"eeg_channels", {1, 2, 3, 4}},
        {"eeg_names", "TP9,AF7,AF8,TP10"},
        {"other_channels", {5}}
    };
    brainflow_boards_json["boards"]["21"]["auxiliary"] =
    {
        {"name", "MuseSBLEDAux"},
        {"sampling_rate", 52},
        {"timestamp_channel", 7},
        {"marker_channel", 8},
        {"package_num_channel", 0},
        {"num_rows", 9},
        {"accel_channels", {1, 2, 3}},
        {"gyro_channels", {4, 5, 6}}
    };
    brainflow_boards_json["boards"]["21"]["ancillary"] =
    {
        {"name", "MuseSBLEDAnc"},
        {"sampling_rate", 64},
        {"timestamp_channel", 4},
        {"marker_channel", 5},
        {"package_num_channel", 0},
        {"num_rows", 6},
        {"ppg_channels", {1, 2, 3}}
    };
    brainflow_boards_json["boards"]["22"]["default"] =
    {
        {"name", "Muse2BLED"},
        {"sampling_rate", 256},
        {"timestamp_channel", 6},
        {"marker_channel", 7},
        {"package_num_channel", 0},
        {"num_rows", 8},
        {"eeg_channels", {1, 2, 3, 4}},
        {"eeg_names", "TP9,AF7,AF8,TP10"},
        {"other_channels", {5}}
    };
    brainflow_boards_json["boards"]["22"]["auxiliary"] =
    {
        {"name", "Muse2BLEDAux"},
        {"sampling_rate", 52},
        {"timestamp_channel", 7},
        {"marker_channel", 8},
        {"package_num_channel", 0},
        {"num_rows", 9},
        {"accel_channels", {1, 2, 3}},
        {"gyro_channels", {4, 5, 6}}
    };
    brainflow_boards_json["boards"]["22"]["ancillary"] =
    {
        {"name", "Muse2BLEDAnc"},
        {"sampling_rate", 64},
        {"timestamp_channel", 4},
        {"marker_channel", 5},
        {"package_num_channel", 0},
        {"num_rows", 6},
        {"ppg_channels", {1, 2, 3}}
    };
    brainflow_boards_json["boards"]["23"]["default"] =
    {
        {"name", "CrownOSC"},
        {"sampling_rate", 256},
        {"timestamp_channel", 10},
        {"marker_channel", 11},
        {"package_num_channel", 0},
        {"num_rows", 12},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"eeg_names", "CP3,C3,F5,PO3,PO4,F6,C4,CP4"},
        {"other_channels", {9}}
    };
    brainflow_boards_json["boards"]["24"]["default"] =
    {
        {"name", "AntNeuroEE410"},
        {"sampling_rate", 2000},
        {"timestamp_channel", 12},
        {"marker_channel", 13},
        {"package_num_channel", 0},
        {"num_rows", 14},
        {"emg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"resistance_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}},
        {"other_channels", {11}}
    };
    brainflow_boards_json["boards"]["25"]["default"] =
    {
        {"name", "AntNeuroEE411"},
        {"sampling_rate", 2000},
        {"timestamp_channel", 12},
        {"marker_channel", 13},
        {"package_num_channel", 0},
        {"num_rows", 14},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"resistance_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}},
        {"other_channels", {11}}
    };
    brainflow_boards_json["boards"]["26"]["default"] =
    {
        {"name", "AntNeuroEE430"},
        {"sampling_rate", 512},
        {"timestamp_channel", 12},
        {"marker_channel", 13},
        {"package_num_channel", 0},
        {"num_rows", 14},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"resistance_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}},
        {"other_channels", {11}}
    };
    brainflow_boards_json["boards"]["27"]["default"] =
    {
        {"name", "AntNeuroEE211"},
        {"sampling_rate", 2000},
        {"timestamp_channel", 68},
        {"marker_channel", 69},
        {"package_num_channel", 0},
        {"num_rows", 70},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64}},
        {"resistance_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66}},
        {"other_channels", {67}}
    };
    brainflow_boards_json["boards"]["28"]["default"] =
    {
        {"name", "AntNeuroEE212"},
        {"sampling_rate", 2000},
        {"timestamp_channel", 36},
        {"marker_channel", 37},
        {"package_num_channel", 0},
        {"num_rows", 38},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32}},
        {"resistance_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34}},
        {"other_channels", {35}}
    };
    brainflow_boards_json["boards"]["29"]["default"] =
    {
        {"name", "AntNeuroEE213"},
        {"sampling_rate", 2000},
        {"timestamp_channel", 20},
        {"marker_channel", 21},
        {"package_num_channel", 0},
        {"num_rows", 22},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
        {"resistance_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18}},
        {"other_channels", {19}}
    };
    brainflow_boards_json["boards"]["30"]["default"] =
    {
        {"name", "AntNeuroEE214"},
        {"sampling_rate", 2000},
        {"timestamp_channel", 60},
        {"marker_channel", 61},
        {"package_num_channel", 0},
        {"num_rows", 62},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32}},
        {"emg_channels", {33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56}},
        {"resistance_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58}},
        {"other_channels", {59}}
    };
    brainflow_boards_json["boards"]["31"]["default"] =
    {
        {"name", "AntNeuroEE215"},
        {"sampling_rate", 2000},
        {"timestamp_channel", 92},
        {"marker_channel", 93},
        {"package_num_channel", 0},
        {"num_rows", 94},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64}},
        {"emg_channels", {65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88}},
        {"resistance_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 55, 56, 57, 58, 59, 60, 61, 62, 63,
            64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90}},
        {"other_channels", {91}}
    };
    brainflow_boards_json["boards"]["32"]["default"] =
    {
        {"name", "AntNeuroEE221"},
        {"sampling_rate", 16000},
        {"timestamp_channel", 20},
        {"marker_channel", 21},
        {"package_num_channel", 0},
        {"num_rows", 22},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}},
        {"resistance_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18}},
        {"other_channels", {19}}
    };
    brainflow_boards_json["boards"]["33"]["default"] =
    {
        {"name", "AntNeuroEE222"},
        {"sampling_rate", 16000},
        {"timestamp_channel", 36},
        {"marker_channel", 37},
        {"package_num_channel", 0},
        {"num_rows", 38},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32}},
        {"resistance_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34}},
        {"other_channels", {35}}
    };
    brainflow_boards_json["boards"]["34"]["default"] =
    {
        {"name", "AntNeuroEE223"},
        {"sampling_rate", 16000},
        {"timestamp_channel", 60},
        {"marker_channel", 61},
        {"package_num_channel", 0},
        {"num_rows", 62},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32}},
        {"emg_channels", {33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56}},
        {"resistance_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58}},
        {"other_channels", {59}}
    };
    brainflow_boards_json["boards"]["35"]["default"] =
    {
        {"name", "AntNeuroEE224"},
        {"sampling_rate", 16000},
        {"timestamp_channel", 68},
        {"marker_channel", 69},
        {"package_num_channel", 0},
        {"num_rows", 70},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64}},
        {"resistance_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66}},
        {"other_channels", {67}}
    };
    brainflow_boards_json["boards"]["36"]["default"] =
    {
        {"name", "AntNeuroEE225"},
        {"sampling_rate", 16000},
        {"timestamp_channel", 92},
        {"marker_channel", 93},
        {"package_num_channel", 0},
        {"num_rows", 94},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64}},
        {"emg_channels", {65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88}},
        {"resistance_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 55, 56, 57, 58, 59, 60, 61, 62, 63,
            64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90}},
        {"other_channels", {91}}
    };
    brainflow_boards_json["boards"]["37"]["default"] =
    {
        {"name", "Enophone"},
        {"sampling_rate", 250},
        {"timestamp_channel", 5},
        {"marker_channel", 6},
        {"package_num_channel", 0},
        {"num_rows", 7},
        {"eeg_channels", {1, 2, 3, 4}},
        {"eeg_names", "A2,A1,C4,C3"}
    };
    brainflow_boards_json["boards"]["38"]["default"] =
    {
        {"name", "Muse2"},
        {"sampling_rate", 256},
        {"timestamp_channel", 6},
        {"marker_channel", 7},
        {"package_num_channel", 0},
        {"num_rows", 8},
        {"eeg_channels", {1, 2, 3, 4}},
        {"eeg_names", "TP9,AF7,AF8,TP10"},
        {"other_channels", {5}}
    };
    brainflow_boards_json["boards"]["38"]["auxiliary"] =
    {
        {"name", "Muse2Aux"},
        {"sampling_rate", 52},
        {"timestamp_channel", 7},
        {"marker_channel", 8},
        {"package_num_channel", 0},
        {"num_rows", 9},
        {"accel_channels", {1, 2, 3}},
        {"gyro_channels", {4, 5, 6}}
    };
    brainflow_boards_json["boards"]["38"]["ancillary"] =
    {
        {"name", "Muse2Anc"},
        {"sampling_rate", 64},
        {"timestamp_channel", 4},
        {"marker_channel", 5},
        {"package_num_channel", 0},
        {"num_rows", 6},
        {"ppg_channels", {1, 2, 3}}
    };
    brainflow_boards_json["boards"]["39"]["default"] =
    {
        {"name", "MuseS"},
        {"sampling_rate", 256},
        {"timestamp_channel", 6},
        {"marker_channel", 7},
        {"package_num_channel", 0},
        {"num_rows", 8},
        {"eeg_channels", {1, 2, 3, 4}},
        {"eeg_names", "TP9,AF7,AF8,TP10"},
        {"other_channels", {5}}
    };
    brainflow_boards_json["boards"]["39"]["auxiliary"] =
    {
        {"name", "MuseSAux"},
        {"sampling_rate", 52},
        {"timestamp_channel", 7},
        {"marker_channel", 8},
        {"package_num_channel", 0},
        {"num_rows", 9},
        {"accel_channels", {1, 2, 3}},
        {"gyro_channels", {4, 5, 6}}
    };
    brainflow_boards_json["boards"]["39"]["ancillary"] =
    {
        {"name", "MuseSAnc"},
        {"sampling_rate", 64},
        {"timestamp_channel", 4},
        {"marker_channel", 5},
        {"package_num_channel", 0},
        {"num_rows", 6},
        {"ppg_channels", {1, 2, 3}}
    };
    brainflow_boards_json["boards"]["40"]["default"] =
    {
        {"name", "BrainAlive"},
        {"sampling_rate", 250},
        {"marker_channel", 15},
        {"timestamp_channel", 16},
        {"num_rows", 17},
        {"package_num_channel", 0},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"eeg_names", "F7,FT7,T7,CP5,CZ,C6,FC6,F4"},
        {"accel_channels", {9, 10, 11}},
        {"gyro_channels", {12, 13, 14}}
    };
    brainflow_boards_json["boards"]["41"]["default"] =
    {
        {"name", "Muse2016"},
        {"sampling_rate", 256},
        {"timestamp_channel", 5},
        {"marker_channel", 6},
        {"package_num_channel", 0},
        {"num_rows", 7},
        {"eeg_channels", {1, 2, 3, 4}},
        {"eeg_names", "TP9,AF7,AF8,TP10"}
    };
    brainflow_boards_json["boards"]["41"]["auxiliary"] =
    {
        {"name", "Muse2016Aux"},
        {"sampling_rate", 52},
        {"timestamp_channel", 7},
        {"marker_channel", 8},
        {"package_num_channel", 0},
        {"num_rows", 9},
        {"accel_channels", {1, 2, 3}},
        {"gyro_channels", {4, 5, 6}}
    };
    brainflow_boards_json["boards"]["42"]["default"] =
    {
        {"name", "Muse2016BLED"},
        {"sampling_rate", 256},
        {"timestamp_channel", 5},
        {"marker_channel", 6},
        {"package_num_channel", 0},
        {"num_rows", 7},
        {"eeg_channels", {1, 2, 3, 4}},
        {"eeg_names", "TP9,AF7,AF8,TP10"},
    };
    brainflow_boards_json["boards"]["42"]["auxiliary"] =
    {
        {"name", "Muse2016BLEDAux"},
        {"sampling_rate", 52},
        {"timestamp_channel", 7},
        {"marker_channel", 8},
        {"package_num_channel", 0},
        {"num_rows", 9},
        {"accel_channels", {1, 2, 3}},
        {"gyro_channels", {4, 5, 6}}
    };
    brainflow_boards_json["boards"]["44"]["default"] =
    {
        {"name", "Explore4Channels"},
        {"sampling_rate", 250},
        {"timestamp_channel", 6},
        {"marker_channel", 7},
        {"package_num_channel", 0},
        {"num_rows", 8},
        {"eeg_channels", {1, 2, 3, 4}},
        {"emg_channels", {1, 2, 3, 4}},
        {"ecg_channels", {1, 2, 3, 4}},
        {"other_channels", {5}} // data status
    };
    brainflow_boards_json["boards"]["44"]["auxiliary"] =
    {
        {"name", "Explore4Channels"},
        {"sampling_rate", 20},
        {"timestamp_channel", 10},
        {"marker_channel", 11},
        {"package_num_channel", 0},
        {"num_rows", 12},
        {"accel_channels", {1, 2, 3}},
        {"gyro_channels", {4, 5, 6}},
        {"magnetometer_channels", {7, 8, 9}}
    };
    brainflow_boards_json["boards"]["44"]["ancillary"] =
    {
        {"name", "Explore4Channels"},
        {"sampling_rate", 1},
        {"timestamp_channel", 4},
        {"marker_channel", 5},
        {"package_num_channel", 0},
        {"num_rows", 6},
        {"temperature_channels", {1}},
        {"battery_channel", 2},
        {"other_channels", {3}}
    };
    brainflow_boards_json["boards"]["45"]["default"] =
    {
        {"name", "Explore8Channels"},
        {"sampling_rate", 250},
        {"timestamp_channel", 10},
        {"marker_channel", 11},
        {"package_num_channel", 0},
        {"num_rows", 12},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"emg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"ecg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"other_channels", {9}} // data status
    };
    brainflow_boards_json["boards"]["45"]["auxiliary"] =
    {
        {"name", "Explore8Channels"},
        {"sampling_rate", 20},
        {"timestamp_channel", 10},
        {"marker_channel", 11},
        {"package_num_channel", 0},
        {"num_rows", 12},
        {"accel_channels", {1, 2, 3}},
        {"gyro_channels", {4, 5, 6}},
        {"magnetometer_channels", {7, 8, 9}}
    };
    brainflow_boards_json["boards"]["45"]["ancillary"] =
    {
        {"name", "Explore8Channels"},
        {"sampling_rate", 1},
        {"timestamp_channel", 4},
        {"marker_channel", 5},
        {"package_num_channel", 0},
        {"num_rows", 6},
        {"temperature_channels", {1}},
        {"battery_channel", 2},
        {"other_channels", {3}}
    };
    brainflow_boards_json["boards"]["46"]["default"] =
    {
        {"name", "Ganglion"},
        {"sampling_rate", 200},
        {"package_num_channel", 0},
        {"timestamp_channel", 13},
        {"marker_channel", 14},
        {"num_rows", 15},
        {"eeg_channels", {1, 2, 3, 4}},
        {"emg_channels", {1, 2, 3, 4}},
        {"ecg_channels", {1, 2, 3, 4}},
        {"eog_channels", {1, 2, 3, 4}},
        {"accel_channels", {5, 6, 7}},
        {"resistance_channels", {8, 9, 10, 11, 12}}
    };
    // todo add other data types and check/fix sampling rates for them
    brainflow_boards_json["boards"]["47"]["default"] =
    {
        {"name", "Emotibit"},
        {"sampling_rate", 25}, // random value for now
        {"package_num_channel", 0},
        {"timestamp_channel", 10},
        {"marker_channel", 11},
        {"num_rows", 12},
        {"accel_channels", {1, 2, 3}},
        {"gyro_channels", {4, 5, 6}},
        {"magnetometer_channels", {7, 8, 9}}
    };
    brainflow_boards_json["boards"]["47"]["auxiliary"] =
    {
        {"name", "Emotibit"},
        {"sampling_rate", 25},
        {"package_num_channel", 0},
        {"timestamp_channel", 4},
        {"marker_channel", 5},
        {"num_rows", 6},
        {"ppg_channels", {1, 2, 3}}
    };
    brainflow_boards_json["boards"]["47"]["ancillary"] =
    {
        {"name", "Emotibit"},
        {"sampling_rate", 15},
        {"package_num_channel", 0},
        {"timestamp_channel", 4},
        {"marker_channel", 5},
        {"num_rows", 6},
        {"eda_channels", {1}},
        {"temperature_channels", {2}},
        {"other_channels", {3}}
    };
    brainflow_boards_json["boards"]["48"]["default"] =
    {
        {"name", "GaleaV4"},
        {"sampling_rate", 250},
        {"package_num_channel", 0},
        {"timestamp_channel", 27},
        {"marker_channel", 28},
        {"num_rows", 29},
        {"eeg_channels", {9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24}},
        {"eeg_names", "F1,C3,F2,Cz,C4,Pz,P4,O2,P3,O1,X1,X2,X3,X4,X5,X6"},
        {"emg_channels", {1, 2, 3, 4, 7, 8}},
        {"eog_channels", {5, 6}},
        {"other_channels", {25, 26}},
    };
    brainflow_boards_json["boards"]["48"]["auxiliary"] =
    {
        {"name", "GaleaV4Auxiliary"},
        {"sampling_rate", 50},
        {"package_num_channel", 0},
        {"timestamp_channel", 17},
        {"marker_channel", 18},
        {"num_rows", 19},
        {"battery_channel", 5},
        {"eda_channels", {1}},
        {"ppg_channels", {2, 3}},
        {"accel_channels", {6, 7, 8}},
        {"gyro_channels", {9, 10, 11}},
        {"magnetometer_channels", {12, 13, 14}},
        {"other_channels", {15, 16}},
        {"temperature_channels", {4}}
    };
    brainflow_boards_json["boards"]["49"]["default"] =
    {
        {"name", "GaleaSerial"},
        {"sampling_rate", 250},
        {"package_num_channel", 0},
        {"timestamp_channel", 27},
        {"marker_channel", 28},
        {"num_rows", 29},
        {"eeg_channels", {9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24}},
        {"eeg_names", "F1,C3,F2,Cz,C4,Pz,P4,O2,P3,O1,X1,X2,X3,X4,X5,X6"},
        {"emg_channels", {1, 2, 3, 4, 7, 8}},
        {"eog_channels", {5, 6}},
        {"other_channels", {25, 26}},
    };
    brainflow_boards_json["boards"]["49"]["auxiliary"] =
    {
        {"name", "GaleaSerialAuxiliary"},
        {"sampling_rate", 50},
        {"package_num_channel", 0},
        {"timestamp_channel", 17},
        {"marker_channel", 18},
        {"num_rows", 19},
        {"battery_channel", 5},
        {"eda_channels", {1}},
        {"ppg_channels", {2, 3}},
        {"accel_channels", {6, 7, 8}},
        {"gyro_channels", {9, 10, 11}},
        {"magnetometer_channels", {12, 13, 14}},
        {"other_channels", {15, 16}},
        {"temperature_channels", {4}}
    };
    brainflow_boards_json["boards"]["50"]["default"] =
    {
        {"name", "NtlWifi"},
        {"sampling_rate", 250},
        {"package_num_channel", 0},
        {"timestamp_channel", 23},
        {"marker_channel", 24},
        {"num_rows", 25},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"eeg_names", "Fp1,Fp2,C3,C4,P7,P8,O1,O2"},
        {"emg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"ecg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"eog_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"accel_channels", {9, 10, 11}},
        {"analog_channels", {19, 20, 21}},
        {"other_channels", {12, 13, 14, 15, 16, 17, 18}},
        {"battery_channel", 22}
    };
    brainflow_boards_json["boards"]["51"]["default"] =
    {
        {"name", "AntNeuroEE511"},
        {"sampling_rate", 4096},
        {"timestamp_channel", 30},
        {"marker_channel", 31},
        {"package_num_channel", 0},
        {"num_rows", 32},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24}},
        {"emg_channels", {25, 26, 27, 28}},
        {"other_channels", {29}}
    };
    brainflow_boards_json["boards"]["52"]["default"] =
    {
        {"name", "FreeEEG128"},
        {"sampling_rate", 256},
        {"timestamp_channel", 129},
        {"marker_channel", 130},
        {"package_num_channel", 0},
        {"num_rows", 131},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128}},
        {"emg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128}},
        {"ecg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128}}
    };
    brainflow_boards_json["boards"]["53"]["default"] =
    {
        {"name", "AAVAA V3"},
        {"sampling_rate", 50},
        {"package_num_channel", 0},
        {"timestamp_channel", 15},
        {"marker_channel", 16},
        {"num_rows", 17},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"eeg_names", "L1,L2,L3,L4,R1,R2,R3,R4"},
        {"emg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"ecg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"eog_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"rotation_channels", {9, 10, 11}},
        {"battery_channel", 12},
        {"other_channels", {13, 14}}
    };
    brainflow_boards_json["boards"]["54"]["default"] =
    {
        {"name", "ExplorePlus8Channels"},
        {"sampling_rate", 250},
        {"timestamp_channel", 10},
        {"marker_channel", 11},
        {"package_num_channel", 0},
        {"num_rows", 12},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"emg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"ecg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"other_channels", {9}} // data status
    };
    brainflow_boards_json["boards"]["54"]["auxiliary"] =
    {
        {"name", "ExplorePlus8Channels"},
        {"sampling_rate", 20},
        {"timestamp_channel", 10},
        {"marker_channel", 11},
        {"package_num_channel", 0},
        {"num_rows", 12},
        {"accel_channels", {1, 2, 3}},
        {"gyro_channels", {4, 5, 6}},
        {"magnetometer_channels", {7, 8, 9}}
    };
    brainflow_boards_json["boards"]["54"]["ancillary"] =
    {
        {"name", "ExplorePlus8Channels"},
        {"sampling_rate", 1},
        {"timestamp_channel", 4},
        {"marker_channel", 5},
        {"package_num_channel", 0},
        {"num_rows", 6},
        {"temperature_channels", {1}},
        {"battery_channel", 2},
        {"other_channels", {3}}
    };
    brainflow_boards_json["boards"]["55"]["default"] =
    {
        {"name", "ExplorePlus32Channels"},
        {"sampling_rate", 250},
        {"timestamp_channel", 34},
        {"marker_channel", 35},
        {"package_num_channel", 0},
        {"num_rows", 36},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32}},
        {"emg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32}},
        {"ecg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32}},
        {"other_channels", {33}} // data status
    };
    brainflow_boards_json["boards"]["55"]["auxiliary"] =
    {
        {"name", "ExplorePlus32Channels"},
        {"sampling_rate", 20},
        {"timestamp_channel", 10},
        {"marker_channel", 11},
        {"package_num_channel", 0},
        {"num_rows", 12},
        {"accel_channels", {1, 2, 3}},
        {"gyro_channels", {4, 5, 6}},
        {"magnetometer_channels", {7, 8, 9}}
    };
    brainflow_boards_json["boards"]["55"]["ancillary"] =
    {
        {"name", "ExplorePlus32Channels"},
        {"sampling_rate", 1},
        {"timestamp_channel", 4},
        {"marker_channel", 5},
        {"package_num_channel", 0},
        {"num_rows", 6},
        {"temperature_channels", {1}},
        {"battery_channel", 2},
        {"other_channels", {3}}
    };
    brainflow_boards_json["boards"]["56"]["default"] = {
        {"name", "PIEEG"},
        {"sampling_rate", 250},
        {"package_num_channel", 0},
        {"timestamp_channel", 9},
        {"marker_channel", 10},
        {"num_rows", 11},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},  
        {"eeg_names", "Fp1,Fp2,C3,C4,P7,P8,O1,O2"}
    };
    brainflow_boards_json["boards"]["57"]["default"] =
    {
        {"name", "Knight"},
        {"sampling_rate", 125},
        {"timestamp_channel", 11},
        {"marker_channel",12},
        {"package_num_channel", 0},
        {"num_rows", 13},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"other_channels", {9, 10}}
    };
    brainflow_boards_json["boards"]["58"]["default"] = {
        {"name", "Sync-Trio"},
        {"sampling_rate", 250},
        {"package_num_channel", 0},
        {"timestamp_channel", 4},
        {"marker_channel", 5},
        {"num_rows", 6},
        {"eeg_channels", {1, 2}},
        {"ecg_channels", {3}}
    };
    brainflow_boards_json["boards"]["59"]["default"] = {
        {"name", "Sync-Octo"},
        {"sampling_rate", 250},
        {"package_num_channel", 0},
        {"timestamp_channel", 9},
        {"marker_channel", 10},
        {"num_rows", 11},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7}},
        {"ecg_channels", {8}}
    };
        brainflow_boards_json["boards"]["60"]["default"] = {
        {"name", "OB5000MAX"},
        {"sampling_rate", 250},
        {"package_num_channel", 0},
        {"timestamp_channel", 9},
        {"marker_channel", 10},
        {"num_rows", 11},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
    };
    brainflow_boards_json["boards"]["61"]["default"] = {
        {"name", "Sync-Neo"},
        {"sampling_rate", 250},
        {"package_num_channel", 0},
        {"timestamp_channel", 9},
        {"marker_channel", 10},
        {"num_rows", 11},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8}}
    };
    brainflow_boards_json["boards"]["62"]["default"] = {
        {"name", "Sync-Uno"},
        {"sampling_rate", 250},
        {"package_num_channel", 0},
        {"timestamp_channel", 2},
        {"marker_channel", 3},
        {"num_rows", 4},
        {"eeg_channels", {1}}
    };
    brainflow_boards_json["boards"]["63"]["default"] = {
        {"name", "OB3000"},
        {"sampling_rate", 500},
        {"package_num_channel", 0},
        {"timestamp_channel", 25},
        {"marker_channel", 26},
        {"num_rows", 27},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23}},
        {"ecg_channels", {24}}
    };
    brainflow_boards_json["boards"]["64"]["default"] =
    {
        {"name", "BioListener"},
        {"sampling_rate", 500},
        {"timestamp_channel", 11},
        {"marker_channel", 12},
        {"package_num_channel", 0},
        {"num_rows", 13},
        {"eeg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"emg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"ecg_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"eog_channels", {1, 2, 3, 4, 5, 6, 7, 8}},
        {"other_channels", {9, 10}}
    };
    brainflow_boards_json["boards"]["64"]["auxiliary"] =
    {
        {"name", "BioListener"},
        {"sampling_rate", 50},
        {"timestamp_channel", 11},
        {"marker_channel", 12},
        {"package_num_channel", 0},
        {"num_rows", 13},
        {"accel_channels", {1, 2, 3}},
        {"gyro_channels", {4, 5, 6}},
        {"temperature_channels", {7}},
        {"battery_channel", 8},
        {"other_channels", {9, 10}}
    };
}

BrainFlowBoards boards_struct;

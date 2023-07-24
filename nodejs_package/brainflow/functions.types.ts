import { BoardIds, BrainFlowExitCodes, BrainFlowPresets, LogLevels } from './brainflow.types';

// --------------------------- //
//  C++ Functions
// --------------------------- //

// c-like prototype strings
export enum BoardControllerCLikeFunctions {
  // Simple methods:
  start_stream = 'int start_stream (int buffer_size, const char *streamer_params, int board_id, const char *json_brainflow_input_params)',
  stop_stream = 'int stop_stream (int board_id, const char *json_brainflow_input_params)',
  prepare_session = 'int prepare_session (int board_id, const char *json_brainflow_input_params)',
  release_all_sessions = 'int release_all_sessions ()',
  release_session = 'int release_session (int board_id, const char *json_brainflow_input_params)',
  add_streamer = 'int add_streamer (const char *streamer, int preset, int board_id, const char *json_brainflow_input_params)',
  delete_streamer = 'int delete_streamer (const char *streamer, int preset, int board_id, const char *json_brainflow_input_params)',
  insert_marker = 'int insert_marker (double value, int preset, int board_id, const char *json_brainflow_input_params)',
  set_log_level_board_controller = 'int set_log_level_board_controller (int log_level)',
  set_log_file_board_controller = 'int set_log_file_board_controller (const char *log_file)',
  log_message_board_controller = 'int log_message_board_controller (int log_level, char *log_message)',

  // Methods with '_Inout_' pointer -> dual input/output parameter:
  is_prepared = 'int is_prepared (_Inout_ int *prepared, int board_id, const char *json_brainflow_input_params)',
  get_board_data_count = 'int get_board_data_count (int preset, _Inout_ int *result, int board_id, const char *json_brainflow_input_params)',
  get_board_data = 'int get_board_data (int data_count, int preset, _Inout_ double *data_buf, int board_id, const char *json_brainflow_input_params)',
  get_num_rows = 'int get_num_rows (int board_id, int preset, _Inout_ int *num_rows)',
  get_current_board_data = 'int get_current_board_data (int num_samples, int preset, _Inout_ double *data_buf, _Inout_ int *returned_samples, int board_id, const char *json_brainflow_input_params)',
  get_sampling_rate = 'int get_sampling_rate (int board_id, int preset, _Inout_ int *sampling_rate)',
  get_eeg_channels = 'int get_eeg_channels (int board_id, int preset, _Inout_ int *eeg_channels, _Inout_ int *len)',
}

export class BoardControllerFunctions {
  addStreamer!: (
    streamer: string,
    preset: BrainFlowPresets,
    boardId: BoardIds,
    inputJson: string,
  ) => BrainFlowExitCodes;
  deleteStreamer!: (
    streamer: string,
    preset: BrainFlowPresets,
    boardId: BoardIds,
    inputJson: string,
  ) => BrainFlowExitCodes;
  insertMarker!: (value: number, preset: BrainFlowPresets, boardId: BoardIds, inputJson: string) => BrainFlowExitCodes;
  setLogLevelBoardController!: (logLevel: LogLevels) => BrainFlowExitCodes;
  setLogFileBoardController!: (logFile: string) => BrainFlowExitCodes;
  logMessageBoardController!: (logLevel: LogLevels, logMessage: string) => BrainFlowExitCodes;
  isPrepared!: (prepared: number[], boardId: BoardIds, inputJson: string) => BrainFlowExitCodes;
  prepareSession!: (boardId: BoardIds, inputJson: string) => BrainFlowExitCodes;
  startStream!: (
    numSamples: number,
    streamerParams: string | null,
    boardId: BoardIds,
    inputJson: string,
  ) => BrainFlowExitCodes;
  getBoardDataCount!: (
    preset: BrainFlowPresets,
    dataSize: number[],
    boardId: BoardIds,
    inputJson: string,
  ) => BrainFlowExitCodes;
  getBoardData!: (
    dataSize: number,
    preset: BrainFlowPresets,
    dataArr: number[],
    boardId: BoardIds,
    inputJson: string,
  ) => BrainFlowExitCodes;
  getCurrentBoardData!: (
    numSamples: number,
    preset: BrainFlowPresets,
    dataBuf: number[],
    returnedSamples: number[],
    boardId: BoardIds,
    inputJson: string,
  ) => BrainFlowExitCodes;
  getNumRows!: (boardId: BoardIds, preset: BrainFlowPresets, numRows: number[]) => BrainFlowExitCodes;
  releaseAllSessions!: () => BrainFlowExitCodes;
  releaseSession!: (boardId: BoardIds, inputJson: string) => BrainFlowExitCodes;
  stopStream!: (boardId: BoardIds, inputJson: string) => BrainFlowExitCodes;
  getSamplingRate!: (boardId: BoardIds, preset: BrainFlowPresets, samplingRate: number[]) => BrainFlowExitCodes;
  getEegChannels!: (
    boardId: BoardIds,
    preset: BrainFlowPresets,
    eegChannels: number[],
    len: number[],
  ) => BrainFlowExitCodes;
}

export enum DataHandlerCLikeFunctions {
  // Methods with '_Inout_' pointer -> dual input/output parameter:
  get_railed_percentage = 'int get_railed_percentage (double *raw_data, int data_len, int gain, _Inout_ double *output)',
}

export class DataHandlerFunctions {
  getRailedPercentage!: (rawData: number[], dataLen: number, gain: number, output: number[]) => BrainFlowExitCodes;
}

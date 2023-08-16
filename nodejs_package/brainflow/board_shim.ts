import _ from 'lodash';
import * as os from 'os';
import koffi from 'koffi';
import {
  BoardIds,
  IBrainFlowInputParams,
  IpProtocolTypes,
  BrainFlowPresets,
  BrainFlowExitCodes,
  LogLevels,
  BrainFlowError,
} from './brainflow.types';
import { BoardControllerCLikeFunctions as CLike, BoardControllerFunctions } from './functions.types';

class BrainFlowInputParams {
  private inputParams: IBrainFlowInputParams = {
    serialPort: '',
    macAddress: '',
    ipAddress: '',
    ipAddressAux: '',
    ipAddressAnc: '',
    ipPort: 0,
    ipPortAux: 0,
    ipPortAnc: 0,
    ipProtocol: IpProtocolTypes.NO_IP_PROTOCOL,
    otherInfo: '',
    timeout: 0,
    serialNumber: '',
    file: '',
    fileAux: '',
    fileAnc: '',
    masterBoard: BoardIds.NO_BOARD,
  };

  constructor(inputParams: Partial<IBrainFlowInputParams>) {
    this.inputParams = { ...this.inputParams, ...inputParams };
  }

  public toJson(): string {
    const params: Record<string, any> = {};
    Object.keys(this.inputParams).forEach((key) => {
      params[_.snakeCase(key)] = this.inputParams[key as keyof IBrainFlowInputParams];
    });
    return JSON.stringify(params);
  }
}

class BoardControllerDLL extends BoardControllerFunctions {
  private libPath: string;
  private dllPath: string;
  private lib: koffi.IKoffiLib;

  constructor() {
    super();
    this.libPath = `${__dirname}/lib`;
    this.dllPath = this.getDLLPath();
    this.lib = this.getLib();

    this.isPrepared = this.lib.func(CLike.is_prepared);
    this.prepareSession = this.lib.func(CLike.prepare_session);
    this.startStream = this.lib.func(CLike.start_stream);
    this.getBoardDataCount = this.lib.func(CLike.get_board_data_count);
    this.getBoardData = this.lib.func(CLike.get_board_data);
    this.getCurrentBoardData = this.lib.func(CLike.get_current_board_data);
    this.getNumRows = this.lib.func(CLike.get_num_rows);
    this.releaseAllSessions = this.lib.func(CLike.release_all_sessions);
    this.releaseSession = this.lib.func(CLike.release_session);
    this.stopStream = this.lib.func(CLike.stop_stream);
    this.getSamplingRate = this.lib.func(CLike.get_sampling_rate);
    this.getPackageNumChannel = this.lib.func(CLike.get_package_num_channel);
    this.getTimestampChannel = this.lib.func(CLike.get_timestamp_channel);
    this.getMarkerChannel = this.lib.func(CLike.get_marker_channel);
    this.getBatteryChannel = this.lib.func(CLike.get_battery_channel);
    this.getEegChannels = this.lib.func(CLike.get_eeg_channels);
    this.getExgChannels = this.lib.func(CLike.get_exg_channels);
    this.getEmgChannels = this.lib.func(CLike.get_emg_channels);
    this.getEogChannels = this.lib.func(CLike.get_eog_channels);
    this.getEcgChannels = this.lib.func(CLike.get_ecg_channels);
    this.getPpgChannels = this.lib.func(CLike.get_ppg_channels);
    this.getEdaChannels = this.lib.func(CLike.get_eda_channels);
    this.getAccelChannels = this.lib.func(CLike.get_accel_channels);
    this.getAnalogChannels = this.lib.func(CLike.get_analog_channels);
    this.getGyroChannels = this.lib.func(CLike.get_gyro_channels);
    this.getOtherChannels = this.lib.func(CLike.get_other_channels);
    this.getTemperatureChannels = this.lib.func(CLike.get_temperature_channels);
    this.getResistanceChannels = this.lib.func(CLike.get_resistance_channels);
    this.getMagnetometerChannels = this.lib.func(CLike.get_magnetometer_channels);
    this.addStreamer = this.lib.func(CLike.add_streamer);
    this.deleteStreamer = this.lib.func(CLike.delete_streamer);
    this.insertMarker = this.lib.func(CLike.insert_marker);
    this.setLogLevelBoardController = this.lib.func(CLike.set_log_level_board_controller);
    this.setLogFileBoardController = this.lib.func(CLike.set_log_file_board_controller);
    this.logMessageBoardController = this.lib.func(CLike.log_message_board_controller);
  }

  private getDLLPath() {
    const platform = os.platform();
    const arch = os.arch();
    switch (platform) {
      case 'darwin':
        return `${this.libPath}/libBoardController.dylib`;
      case 'win32':
        return arch === 'x64' ? `${this.libPath}/BoardController.dll` : `${this.libPath}/BoardController32.dll`;
      case 'linux':
        return `${this.libPath}/libBoardController.so`;
      default:
        throw new BrainFlowError(BrainFlowExitCodes.GENERAL_ERROR, `OS ${platform} is not supported.`);
    }
  }

  private getLib() {
    try {
      const lib = koffi.load(this.dllPath);
      return lib;
    } catch (err) {
      console.error(err);
      throw new BrainFlowError(BrainFlowExitCodes.GENERAL_ERROR, `${'Could not load BoardController DLL - path://'}${this.dllPath}`);
    }
  }
}

export class BoardShim {
  private boardId: BoardIds;

  private masterBoardId: BoardIds;

  private inputJson: string;

  private boardController: BoardControllerDLL;

  constructor(boardId: BoardIds, inputParams: Partial<IBrainFlowInputParams>) {
    this.boardId = boardId;
    this.masterBoardId =
      inputParams.masterBoard && inputParams.masterBoard !== BoardIds.NO_BOARD ? inputParams.masterBoard : boardId;
    this.inputJson = new BrainFlowInputParams(inputParams).toJson();
    this.boardController = new BoardControllerDLL();
  }

  public isPrepared(): boolean {
    const prepared = [0];
    const res = this.boardController.isPrepared(prepared, this.boardId, this.inputJson);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not check prepared state');
    }
    return !!prepared[0];
  }

  public addStreamer(streamerParams: string, preset = BrainFlowPresets.DEFAULT_PRESET): void {
    const res = this.boardController.addStreamer(streamerParams, preset, this.boardId, this.inputJson);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not add streamer');
    }
  }

  public deleteStreamer(streamerParams: string, preset = BrainFlowPresets.DEFAULT_PRESET): void {
    const res = this.boardController.deleteStreamer(streamerParams, preset, this.boardId, this.inputJson);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not delete streamer');
    }
  }

  public insertMarker(value: number, preset = BrainFlowPresets.DEFAULT_PRESET): void {
    const res = this.boardController.insertMarker(value, preset, this.boardId, this.inputJson);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not insert marker');
    }
  }

  public setLogLevel(logLevel: LogLevels): void {
    const res = this.boardController.setLogLevelBoardController(logLevel);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not set log level properly');
    }
  }

  public setLogFile(file: string): void {
    const res = this.boardController.setLogFileBoardController(file);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not redirect to log file');
    }
  }

  public logMessage(logLevel: LogLevels, message: string): void {
    const res = this.boardController.logMessageBoardController(logLevel, message);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not writte message');
    }
  }

  public prepareSession(): void {
    const res = this.boardController.prepareSession(this.boardId, this.inputJson);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not prepare session');
    }
  }

  public startStream(numSamples = 1800 * 250, streamerParams = null): void {
    const res = this.boardController.startStream(numSamples, streamerParams, this.boardId, this.inputJson);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not start stream');
    }
  }

  public getBoardDataCount(preset = BrainFlowPresets.DEFAULT_PRESET): number {
    const dataSize = [0];
    const res = this.boardController.getBoardDataCount(preset, dataSize, this.boardId, this.inputJson);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not get board data count');
    }
    return dataSize[0];
  }

  public getBoardData(numSamples?: number, preset = BrainFlowPresets.DEFAULT_PRESET): number[][] {
    let dataSize = this.getBoardDataCount(preset);
    if (numSamples) {
      if (numSamples < 1) throw new Error('invalid num_samples');
      dataSize = Math.min(numSamples, dataSize);
    }
    const packageLength = this.getNumRows(this.masterBoardId, preset);
    const dataArr = [...new Array(packageLength * dataSize).fill(0)];
    const res = this.boardController.getBoardData(dataSize, preset, dataArr, this.boardId, this.inputJson);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not get board data');
    }
    return _.chunk(dataArr, dataSize);
  }

  public getCurrentBoardData(numSamples: number, preset = BrainFlowPresets.DEFAULT_PRESET): number[][] | null {
    const packageLength = this.getNumRows(this.masterBoardId, preset);
    const dataArr = [...new Array(packageLength * numSamples).fill(0)];
    const currentSize = [0];
    const res = this.boardController.getCurrentBoardData(
      numSamples,
      preset,
      dataArr,
      currentSize,
      this.boardId,
      this.inputJson,
    );
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not get current board data');
    }
    if (!currentSize.length) {
      return null;
    }
    return _.chunk(dataArr, currentSize[0]);
  }

  public releaseAllSessions(): void {
    const res = this.boardController.releaseAllSessions();
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not release all sessions');
    }
  }

  public releaseSession(): void {
    const res = this.boardController.releaseSession(this.boardId, this.inputJson);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not release session');
    }
  }

  public stopStream(): void {
    const res = this.boardController.stopStream(this.boardId, this.inputJson);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not stop stream');
    }
  }

  public getNumRows(boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number {
    const numRows = [0];
    const res = this.boardController.getNumRows(boardId, preset, numRows);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not get num rows');
    }
    return numRows[0];
  }

  public getPackageNumChannel(boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number {
    const value = [0];
    const res = this.boardController.getPackageNumChannel(boardId, preset, value);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not get such data from this device');
    }
    return value[0];
  }

  public getTimestampChannel(boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number {
    const value = [0];
    const res = this.boardController.getTimestampChannel(boardId, preset, value);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not get such data from this device');
    }
    return value[0];
  }

  public getMarkerChannel(boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number {
    const value = [0];
    const res = this.boardController.getMarkerChannel(boardId, preset, value);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not get such data from this device');
    }
    return value[0];
  }

  public getBatteryChannel(boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number {
    const value = [0];
    const res = this.boardController.getBatteryChannel(boardId, preset, value);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not get such data from this device');
    }
    return value[0];
  }

  public getSamplingRate(boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number {
    const samplingRate = [0];
    const res = this.boardController.getSamplingRate(boardId, preset, samplingRate);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not get sampling rate');
    }
    return samplingRate[0];
  }

  public getEegChannels(boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[] {
    const numChannels = [0];
    const eegChannels = [...new Array(512).fill(0)];
    const res = this.boardController.getEegChannels(boardId, preset, eegChannels, numChannels);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not get board info');
    }
    return eegChannels.slice(0, numChannels[0]);
  }

  public getExgChannels(boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[] {
    const numChannels = [0];
    const сhannels = [...new Array(512).fill(0)];
    const res = this.boardController.getExgChannels(boardId, preset, сhannels, numChannels);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not get board info');
    }
    return сhannels.slice(0, numChannels[0]);
  }

  public getEmgChannels(boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[] {
    const numChannels = [0];
    const сhannels = [...new Array(512).fill(0)];
    const res = this.boardController.getEmgChannels(boardId, preset, сhannels, numChannels);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not get board info');
    }
    return сhannels.slice(0, numChannels[0]);
  }

  public getEcgChannels(boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[] {
    const numChannels = [0];
    const сhannels = [...new Array(512).fill(0)];
    const res = this.boardController.getEcgChannels(boardId, preset, сhannels, numChannels);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not get board info');
    }
    return сhannels.slice(0, numChannels[0]);
  }

  public getEogChannels(boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[] {
    const numChannels = [0];
    const сhannels = [...new Array(512).fill(0)];
    const res = this.boardController.getEogChannels(boardId, preset, сhannels, numChannels);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not get board info');
    }
    return сhannels.slice(0, numChannels[0]);
  }

  public getPpgChannels(boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[] {
    const numChannels = [0];
    const сhannels = [...new Array(512).fill(0)];
    const res = this.boardController.getPpgChannels(boardId, preset, сhannels, numChannels);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not get board info');
    }
    return сhannels.slice(0, numChannels[0]);
  }

  public getEdaChannels(boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[] {
    const numChannels = [0];
    const сhannels = [...new Array(512).fill(0)];
    const res = this.boardController.getEdaChannels(boardId, preset, сhannels, numChannels);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not get board info');
    }
    return сhannels.slice(0, numChannels[0]);
  }

  public getAccelChannels(boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[] {
    const numChannels = [0];
    const сhannels = [...new Array(512).fill(0)];
    const res = this.boardController.getAccelChannels(boardId, preset, сhannels, numChannels);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not get board info');
    }
    return сhannels.slice(0, numChannels[0]);
  }

  public getAnalogChannels(boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[] {
    const numChannels = [0];
    const сhannels = [...new Array(512).fill(0)];
    const res = this.boardController.getAnalogChannels(boardId, preset, сhannels, numChannels);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not get board info');
    }
    return сhannels.slice(0, numChannels[0]);
  }

  public getGyroChannels(boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[] {
    const numChannels = [0];
    const сhannels = [...new Array(512).fill(0)];
    const res = this.boardController.getGyroChannels(boardId, preset, сhannels, numChannels);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not get board info');
    }
    return сhannels.slice(0, numChannels[0]);
  }

  public getOtherChannels(boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[] {
    const numChannels = [0];
    const сhannels = [...new Array(512).fill(0)];
    const res = this.boardController.getOtherChannels(boardId, preset, сhannels, numChannels);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not get board info');
    }
    return сhannels.slice(0, numChannels[0]);
  }

  public getTemperatureChannels(boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[] {
    const numChannels = [0];
    const сhannels = [...new Array(512).fill(0)];
    const res = this.boardController.getTemperatureChannels(boardId, preset, сhannels, numChannels);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not get board info');
    }
    return сhannels.slice(0, numChannels[0]);
  }

  public getResistanceChannels(boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[] {
    const numChannels = [0];
    const сhannels = [...new Array(512).fill(0)];
    const res = this.boardController.getResistanceChannels(boardId, preset, сhannels, numChannels);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not get board info');
    }
    return сhannels.slice(0, numChannels[0]);
  }

  public getMagnetometerChannels(boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[] {
    const numChannels = [0];
    const сhannels = [...new Array(512).fill(0)];
    const res = this.boardController.getMagnetometerChannels(boardId, preset, сhannels, numChannels);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not get board info');
    }
    return сhannels.slice(0, numChannels[0]);
  }

}

import koffi from 'koffi';
import _ from 'lodash';
import * as os from 'os';

import {
    BoardIds,
    BrainFlowError,
    BrainFlowExitCodes,
    BrainFlowPresets,
    IBrainFlowInputParams,
    IpProtocolTypes,
    LogLevels,
} from './brainflow.types';
import {BoardControllerCLikeFunctions as CLike, BoardControllerFunctions} from './functions.types';

export class BrainFlowInputParams
{
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

    constructor(inputParams: Partial<IBrainFlowInputParams>)
    {
        this.inputParams = {...this.inputParams, ...inputParams };
    }

    public toJson(): string
    {
        const params: Record<string, any> = {};
        Object.keys(this.inputParams).forEach((key) => {
            params[_.snakeCase(key)] = this.inputParams[key as keyof IBrainFlowInputParams];
        });
        return JSON.stringify(params);
    }
}

class BoardControllerDLL extends BoardControllerFunctions
{
    private static instance: BoardControllerDLL;

    private libPath: string;
    private dllPath: string;
    private lib: koffi.IKoffiLib;

    private constructor()
    {
        super ();
        this.libPath = `${__dirname}/../brainflow/lib`;
        this.dllPath = this.getDLLPath();
        this.lib = this.getLib();

        this.isPrepared = this.lib.func(CLike.is_prepared);
        this.prepareSession = this.lib.func(CLike.prepare_session);
        this.startStream = this.lib.func(CLike.start_stream);
        this.configBoard = this.lib.func(CLike.config_board);
        this.configBoardWithBytes = this.lib.func(CLike.config_board_with_bytes);
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
        this.getRotationChannels = this.lib.func(CLike.get_rotation_channels);
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
        this.getVersionBoardController = this.lib.func(CLike.get_version_board_controller);
        this.getDeviceName = this.lib.func(CLike.get_device_name);
        this.getBoardPresets = this.lib.func(CLike.get_board_presets);
        this.getEegNames = this.lib.func(CLike.get_eeg_names);
        this.getBoardDescr = this.lib.func(CLike.get_board_descr);
    }

    private getDLLPath()
    {
        const platform = os.platform();
        const arch = os.arch();
        switch (platform)
        {
            case 'darwin':
                return `${this.libPath}/libBoardController.dylib`;
            case 'win32':
                return arch === 'x64' ? `${this.libPath}/BoardController.dll` :
                                        `${this.libPath}/BoardController32.dll`;
            case 'linux':
                return `${this.libPath}/libBoardController.so`;
            default:
                throw new BrainFlowError (
                    BrainFlowExitCodes.GENERAL_ERROR, `OS ${platform} is not supported.`);
        }
    }

    private getLib()
    {
        try
        {
            const lib = koffi.load(this.dllPath);
            return lib;
        }
        catch (err)
        {
            console.error(err);
            throw new BrainFlowError (BrainFlowExitCodes.GENERAL_ERROR,
                `${'Could not load BoardController DLL - path://'}${this.dllPath}`);
        }
    }

    public static getInstance(): BoardControllerDLL
    {
        if (!BoardControllerDLL.instance)
        {
            BoardControllerDLL.instance = new BoardControllerDLL ();
        }
        return BoardControllerDLL.instance;
    }
}

export class BoardShim
{
    private boardId: BoardIds;

    private masterBoardId: BoardIds;

    private inputJson: string;

    constructor(boardId: BoardIds, inputParams: Partial<IBrainFlowInputParams>)
    {
        this.boardId = boardId;
        this.masterBoardId =
            inputParams.masterBoard && inputParams.masterBoard !== BoardIds.NO_BOARD ?
            inputParams.masterBoard :
            boardId;
        this.inputJson = new BrainFlowInputParams (inputParams).toJson();
    }

    // logging methods
    public static getVersion(): string
    {
        const len = [0];
        let out = ['\0'.repeat(512)];
        const res = BoardControllerDLL.getInstance().getVersionBoardController(out, len, 512);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get version info');
        }
        return out[0].substring(0, len[0]);
    }

    public static setLogLevel(logLevel: LogLevels): void
    {
        const res = BoardControllerDLL.getInstance().setLogLevelBoardController(logLevel);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not set log level properly');
        }
    }

    public static setLogFile(file: string): void
    {
        const res = BoardControllerDLL.getInstance().setLogFileBoardController(file);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not redirect to log file');
        }
    }

    public static logMessage(logLevel: LogLevels, message: string): void
    {
        const res = BoardControllerDLL.getInstance().logMessageBoardController(logLevel, message);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not writte message');
        }
    }

    // board methods
    public prepareSession(): void
    {
        const res = BoardControllerDLL.getInstance().prepareSession(this.boardId, this.inputJson);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not prepare session');
        }
    }

    public isPrepared(): boolean
    {
        const prepared = [0];
        const res =
            BoardControllerDLL.getInstance().isPrepared(prepared, this.boardId, this.inputJson);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not check prepared state');
        }
        return !!prepared[0];
    }

    public addStreamer(streamerParams: string, preset = BrainFlowPresets.DEFAULT_PRESET): void
    {
        const res = BoardControllerDLL.getInstance().addStreamer(
            streamerParams, preset, this.boardId, this.inputJson);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not add streamer');
        }
    }

    public deleteStreamer(streamerParams: string, preset = BrainFlowPresets.DEFAULT_PRESET): void
    {
        const res = BoardControllerDLL.getInstance().deleteStreamer(
            streamerParams, preset, this.boardId, this.inputJson);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not delete streamer');
        }
    }

    public insertMarker(value: number, preset = BrainFlowPresets.DEFAULT_PRESET): void
    {
        const res = BoardControllerDLL.getInstance().insertMarker(
            value, preset, this.boardId, this.inputJson);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not insert marker');
        }
    }

    public startStream(numSamples = 1800 * 250, streamerParams = null): void
    {
        const res = BoardControllerDLL.getInstance().startStream(
            numSamples, streamerParams, this.boardId, this.inputJson);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not start stream');
        }
    }

    public configBoard(config: string): string
    {
        const len = [0];
        let out = ['\0'.repeat(4096)];
        const res = BoardControllerDLL.getInstance().configBoard(
            config, out, len, this.boardId, this.inputJson);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not config board');
        }
        return out[0].substring(0, len[0]);
    }

    public configBoardWithBytes(config: string, len: number): void
    {
        const res = BoardControllerDLL.getInstance().configBoardWithBytes(
            config, len, this.boardId, this.inputJson);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not config board with bytes');
        }
    }

    public getBoardDataCount(preset = BrainFlowPresets.DEFAULT_PRESET): number
    {
        const dataSize = [0];
        const res = BoardControllerDLL.getInstance().getBoardDataCount(
            preset, dataSize, this.boardId, this.inputJson);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get board data count');
        }
        return dataSize[0];
    }

    public getBoardData(numSamples?: number, preset = BrainFlowPresets.DEFAULT_PRESET): number[][]
    {
        let dataSize = this.getBoardDataCount(preset);
        if (numSamples)
        {
            if (numSamples < 1)
                throw new Error ('invalid num_samples');
            dataSize = Math.min(numSamples, dataSize);
        }
        const packageLength = BoardShim.getNumRows(this.masterBoardId, preset);
        const dataArr = [...new Array (packageLength * dataSize).fill(0)];
        const res = BoardControllerDLL.getInstance().getBoardData(
            dataSize, preset, dataArr, this.boardId, this.inputJson);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get board data');
        }
        return _.chunk(dataArr, dataSize);
    }

    public getCurrentBoardData(
        numSamples: number, preset = BrainFlowPresets.DEFAULT_PRESET): number[][]
    {
        const packageLength = BoardShim.getNumRows(this.masterBoardId, preset);
        const dataArr = [...new Array (packageLength * numSamples).fill(0)];
        const currentSize = [0];
        const res = BoardControllerDLL.getInstance().getCurrentBoardData(
            numSamples,
            preset,
            dataArr,
            currentSize,
            this.boardId,
            this.inputJson,
        );
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get current board data');
        }
        return _.chunk(dataArr, currentSize[0]);
    }

    public stopStream(): void
    {
        const res = BoardControllerDLL.getInstance().stopStream(this.boardId, this.inputJson);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not stop stream');
        }
    }

    public releaseSession(): void
    {
        const res = BoardControllerDLL.getInstance().releaseSession(this.boardId, this.inputJson);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not release session');
        }
    }

    public static releaseAllSessions(): void
    {
        const res = BoardControllerDLL.getInstance().releaseAllSessions();
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not release all sessions');
        }
    }

    // board info getter methods
    public static getNumRows(boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number
    {
        const numRows = [0];
        const res = BoardControllerDLL.getInstance().getNumRows(boardId, preset, numRows);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get num rows');
        }
        return numRows[0];
    }

    public static getPackageNumChannel(
        boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number
    {
        const value = [0];
        const res = BoardControllerDLL.getInstance().getPackageNumChannel(boardId, preset, value);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get such data from this device');
        }
        return value[0];
    }

    public static getTimestampChannel(
        boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number
    {
        const value = [0];
        const res = BoardControllerDLL.getInstance().getTimestampChannel(boardId, preset, value);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get such data from this device');
        }
        return value[0];
    }

    public static getMarkerChannel(
        boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number
    {
        const value = [0];
        const res = BoardControllerDLL.getInstance().getMarkerChannel(boardId, preset, value);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get such data from this device');
        }
        return value[0];
    }

    public static getBatteryChannel(
        boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number
    {
        const value = [0];
        const res = BoardControllerDLL.getInstance().getBatteryChannel(boardId, preset, value);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get such data from this device');
        }
        return value[0];
    }

    public static getSamplingRate(
        boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number
    {
        const samplingRate = [0];
        const res = BoardControllerDLL.getInstance().getSamplingRate(boardId, preset, samplingRate);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get sampling rate');
        }
        return samplingRate[0];
    }

    public static getEegChannels(
        boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[]
    {
        const numChannels = [0];
        const eegChannels = [...new Array (512).fill(0)];
        const res = BoardControllerDLL.getInstance().getEegChannels(
            boardId, preset, eegChannels, numChannels);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get board info');
        }
        return eegChannels.slice(0, numChannels[0]);
    }

    public static getExgChannels(
        boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[]
    {
        const numChannels = [0];
        const сhannels = [...new Array (512).fill(0)];
        const res =
            BoardControllerDLL.getInstance().getExgChannels(boardId, preset, сhannels, numChannels);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get board info');
        }
        return сhannels.slice(0, numChannels[0]);
    }

    public static getEmgChannels(
        boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[]
    {
        const numChannels = [0];
        const сhannels = [...new Array (512).fill(0)];
        const res =
            BoardControllerDLL.getInstance().getEmgChannels(boardId, preset, сhannels, numChannels);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get board info');
        }
        return сhannels.slice(0, numChannels[0]);
    }

    public static getEcgChannels(
        boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[]
    {
        const numChannels = [0];
        const сhannels = [...new Array (512).fill(0)];
        const res =
            BoardControllerDLL.getInstance().getEcgChannels(boardId, preset, сhannels, numChannels);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get board info');
        }
        return сhannels.slice(0, numChannels[0]);
    }

    public static getEogChannels(
        boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[]
    {
        const numChannels = [0];
        const сhannels = [...new Array (512).fill(0)];
        const res =
            BoardControllerDLL.getInstance().getEogChannels(boardId, preset, сhannels, numChannels);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get board info');
        }
        return сhannels.slice(0, numChannels[0]);
    }

    public static getPpgChannels(
        boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[]
    {
        const numChannels = [0];
        const сhannels = [...new Array (512).fill(0)];
        const res =
            BoardControllerDLL.getInstance().getPpgChannels(boardId, preset, сhannels, numChannels);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get board info');
        }
        return сhannels.slice(0, numChannels[0]);
    }

    public static getEdaChannels(
        boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[]
    {
        const numChannels = [0];
        const сhannels = [...new Array (512).fill(0)];
        const res =
            BoardControllerDLL.getInstance().getEdaChannels(boardId, preset, сhannels, numChannels);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get board info');
        }
        return сhannels.slice(0, numChannels[0]);
    }

    public static getAccelChannels(
        boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[]
    {
        const numChannels = [0];
        const сhannels = [...new Array (512).fill(0)];
        const res = BoardControllerDLL.getInstance().getAccelChannels(
            boardId, preset, сhannels, numChannels);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get board info');
        }
        return сhannels.slice(0, numChannels[0]);
    }

    public static getRotationChannels(
        boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[]
    {
        const numChannels = [0];
        const сhannels = [...new Array (512).fill(0)];
        const res = BoardControllerDLL.getInstance().getRotationChannels(
            boardId, preset, сhannels, numChannels);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get board info');
        }
        return сhannels.slice(0, numChannels[0]);
    }

    public static getAnalogChannels(
        boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[]
    {
        const numChannels = [0];
        const сhannels = [...new Array (512).fill(0)];
        const res = BoardControllerDLL.getInstance().getAnalogChannels(
            boardId, preset, сhannels, numChannels);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get board info');
        }
        return сhannels.slice(0, numChannels[0]);
    }

    public static getGyroChannels(
        boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[]
    {
        const numChannels = [0];
        const сhannels = [...new Array (512).fill(0)];
        const res = BoardControllerDLL.getInstance().getGyroChannels(
            boardId, preset, сhannels, numChannels);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get board info');
        }
        return сhannels.slice(0, numChannels[0]);
    }

    public static getOtherChannels(
        boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[]
    {
        const numChannels = [0];
        const сhannels = [...new Array (512).fill(0)];
        const res = BoardControllerDLL.getInstance().getOtherChannels(
            boardId, preset, сhannels, numChannels);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get board info');
        }
        return сhannels.slice(0, numChannels[0]);
    }

    public static getTemperatureChannels(
        boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[]
    {
        const numChannels = [0];
        const сhannels = [...new Array (512).fill(0)];
        const res = BoardControllerDLL.getInstance().getTemperatureChannels(
            boardId, preset, сhannels, numChannels);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get board info');
        }
        return сhannels.slice(0, numChannels[0]);
    }

    public static getResistanceChannels(
        boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[]
    {
        const numChannels = [0];
        const сhannels = [...new Array (512).fill(0)];
        const res = BoardControllerDLL.getInstance().getResistanceChannels(
            boardId, preset, сhannels, numChannels);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get board info');
        }
        return сhannels.slice(0, numChannels[0]);
    }

    public static getMagnetometerChannels(
        boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): number[]
    {
        const numChannels = [0];
        const сhannels = [...new Array (512).fill(0)];
        const res = BoardControllerDLL.getInstance().getMagnetometerChannels(
            boardId, preset, сhannels, numChannels);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get board info');
        }
        return сhannels.slice(0, numChannels[0]);
    }

    public static getBoardPresets(boardId: BoardIds): number[]
    {
        const len = [0];
        const presets = [...new Array (512).fill(0)];
        const res = BoardControllerDLL.getInstance().getBoardPresets(boardId, presets, len);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get board info');
        }
        return presets.slice(0, len[0]);
    }

    public static getDeviceName(boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): string
    {
        const len = [0];
        let out = ['\0'.repeat(512)];
        const res = BoardControllerDLL.getInstance().getDeviceName(boardId, preset, out, len);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get device info');
        }
        return out[0].substring(0, len[0]);
    }

    public static getEegNames(boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): string[]
    {
        const len = [0];
        let out = ['\0'.repeat(4096)];
        const res = BoardControllerDLL.getInstance().getEegNames(boardId, preset, out, len);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get device info');
        }
        return out[0].substring(0, len[0]).split(",");
    }

    public static getBoardDescr(boardId: BoardIds, preset = BrainFlowPresets.DEFAULT_PRESET): Object
    {
        const len = [0];
        let out = ['\0'.repeat(4096)];
        const res = BoardControllerDLL.getInstance().getBoardDescr(boardId, preset, out, len);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get device info');
        }
        return JSON.parse(out[0].substring(0, len[0]));
    }
}
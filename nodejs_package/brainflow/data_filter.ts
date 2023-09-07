import koffi from 'koffi';
import _ from 'lodash';
import os from 'os';

import {
    AggOperations,
    BrainFlowError,
    BrainFlowExitCodes,
    FilterTypes,
    LogLevels,
    NoiseEstimationLevelTypes,
    NoiseTypes,
    ThresholdTypes,
    WaveletDenoisingTypes,
    WaveletExtensionTypes,
    WaveletTypes
} from './brainflow.types';
import {DataHandlerCLikeFunctions as CLike, DataHandlerFunctions} from './functions.types';

class DataHandlerDLL extends DataHandlerFunctions
{

    private static instance: DataHandlerDLL;

    private libPath: string;
    private dllPath: string;
    private lib: koffi.IKoffiLib;

    private constructor()
    {
        super ();
        this.libPath = `${__dirname}/lib`;
        this.dllPath = this.getDLLPath();
        this.lib = this.getLib();

        this.getRailedPercentage = this.lib.func(CLike.get_railed_percentage);
        this.performLowPass = this.lib.func(CLike.perform_lowpass);
        this.performHighPass = this.lib.func(CLike.perform_highpass);
        this.performBandPass = this.lib.func(CLike.perform_bandpass);
        this.performBandStop = this.lib.func(CLike.perform_bandstop);
        this.removeEnvironmentalNoise = this.lib.func(CLike.remove_environmental_noise);
        this.writeFile = this.lib.func(CLike.write_file);
        this.readFile = this.lib.func(CLike.read_file);
        this.getNumElementsInFile = this.lib.func(CLike.get_num_elements_in_file);
        this.performDownsampling = this.lib.func(CLike.perform_downsampling);
        this.performWaveletTransform = this.lib.func(CLike.perform_wavelet_transform);
        this.performInverseWaveletTransform =
            this.lib.func(CLike.perform_inverse_wavelet_transform);
        this.performWaveletDenoising = this.lib.func(CLike.perform_wavelet_denoising);
        this.getWindow = this.lib.func(CLike.get_window);
        this.performFft = this.lib.func(CLike.perform_fft);
        this.performIfft = this.lib.func(CLike.perform_ifft);
        this.getNearestPowerOfTwo = this.lib.func(CLike.get_nearest_power_of_two);
        this.getPsd = this.lib.func(CLike.get_psd);
        this.getPsdWelch = this.lib.func(CLike.get_psd_welch);
        this.getBandPower = this.lib.func(CLike.get_band_power);
        this.getCustomBandPowers = this.lib.func(CLike.get_custom_band_powers);
        this.getOxygenLevel = this.lib.func(CLike.get_oxygen_level);
        this.getHeartRate = this.lib.func(CLike.get_heart_rate);
        this.restoreDataFromWaveletDetailedCoeffs =
            this.lib.func(CLike.restore_data_from_wavelet_detailed_coeffs);
        this.detectPeaksZScore = this.lib.func(CLike.detect_peaks_z_score);
        this.performIca = this.lib.func(CLike.perform_ica);
        this.getCsp = this.lib.func(CLike.get_csp);
        this.detrend = this.lib.func(CLike.detrend);
        this.calcStddev = this.lib.func(CLike.calc_stddev);
        this.setLogLevelDataHandler = this.lib.func(CLike.set_log_level_data_handler);
        this.setLogFileDataHandler = this.lib.func(CLike.set_log_file_data_handler);
        this.logMessageDataHandler = this.lib.func(CLike.log_message_data_handler);
        this.getVersionDataHandler = this.lib.func(CLike.get_version_data_handler);
    }

    private getDLLPath()
    {
        const platform = os.platform();
        const arch = os.arch();
        switch (platform)
        {
            case 'darwin':
                return `${this.libPath}/libDataHandler.dylib`;
            case 'win32':
                return arch === 'x64' ? `${this.libPath}/DataHandler.dll` :
                                        `${this.libPath}/DataHandler32.dll`;
            case 'linux':
                return `${this.libPath}/libDataHandler.so`;
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
                `${'Could not load DataHandlerDLL DLL - path://'}${this.dllPath}`);
        }
    }

    public static getInstance(): DataHandlerDLL
    {
        if (!DataHandlerDLL.instance)
        {
            DataHandlerDLL.instance = new DataHandlerDLL ();
        }
        return DataHandlerDLL.instance;
    }
}

export class DataFilter
{
    // logging methods
    public static setLogLevel(logLevel: LogLevels): void
    {
        const res = DataHandlerDLL.getInstance().setLogLevelDataHandler(logLevel);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not set log level properly');
        }
    }

    public static setLogFile(file: string): void
    {
        const res = DataHandlerDLL.getInstance().setLogFileDataHandler(file);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not redirect to log file');
        }
    }

    public static logMessage(logLevel: LogLevels, message: string): void
    {
        const res = DataHandlerDLL.getInstance().logMessageDataHandler(logLevel, message);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not writte message');
        }
    }

    public static getVersion(): string
    {
        const len = [0];
        let out = ['\0'.repeat(512)];
        const res = DataHandlerDLL.getInstance().getVersionDataHandler(out, len, 512);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get version info');
        }
        return out[0].substring(0, len[0]);
    }

    // signal processing methods
    public static getRailedPercentage(data: number[], gain: number): number
    {
        const output = [0];
        const res =
            DataHandlerDLL.getInstance().getRailedPercentage(data, data.length, gain, output);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get railed percentage');
        }
        return output[0];
    }

    public static performLowPass(data: number[], samplingRate: number, cutoff: number,
        order: number, filterType: FilterTypes, ripple: number)
    {
        const res = DataHandlerDLL.getInstance().performLowPass(
            data, data.length, samplingRate, cutoff, order, filterType, ripple);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not apply filter');
        }
    }

    public static performHighPass(data: number[], samplingRate: number, cutoff: number,
        order: number, filterType: FilterTypes, ripple: number)
    {
        const res = DataHandlerDLL.getInstance().performHighPass(
            data, data.length, samplingRate, cutoff, order, filterType, ripple);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not apply filter');
        }
    }

    public static performBandPass(data: number[], samplingRate: number, startFreq: number,
        stopFreq: number, order: number, filterType: FilterTypes, ripple: number)
    {
        const res = DataHandlerDLL.getInstance().performBandPass(
            data, data.length, samplingRate, startFreq, stopFreq, order, filterType, ripple);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not apply filter');
        }
    }

    public static performBandStop(data: number[], samplingRate: number, startFreq: number,
        stopFreq: number, order: number, filterType: FilterTypes, ripple: number)
    {
        const res = DataHandlerDLL.getInstance().performBandStop(
            data, data.length, samplingRate, startFreq, stopFreq, order, filterType, ripple);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not apply filter');
        }
    }

    public static removeEnvironmentalNoise(
        data: number[], samplingRate: number, noiseType: NoiseTypes)
    {
        const res = DataHandlerDLL.getInstance().removeEnvironmentalNoise(
            data, data.length, samplingRate, noiseType);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not apply filter');
        }
    }

    public static writeFile(data: number[][], file: string, mode: string)
    {
        if (data.length == 0)
        {
            throw new BrainFlowError (BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR, 'Empty data');
        }
        const flat = data.flat();
        const res =
            DataHandlerDLL.getInstance().writeFile(flat, data.length, data[0].length, file, mode);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not write file');
        }
    }

    public static readFile(file: string): number[][]
    {
        const numElems = DataFilter.getNumElementsInFile(file);
        const dataArr = [...new Array (numElems).fill(0)];
        const rows = [0];
        const cols = [0];
        const res = DataHandlerDLL.getInstance().readFile(dataArr, rows, cols, file, numElems);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not read file');
        }
        return _.chunk(dataArr, cols[0]);
    }

    private static getNumElementsInFile(file: string): number
    {
        const output = [0];
        const res = DataHandlerDLL.getInstance().getNumElementsInFile(file, output);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get info about file');
        }
        return output[0];
    }

    public static performDownsampling(
        data: number[], period: number, aggOperation: AggOperations): number[]
    {
        if (period < 1)
        {
            throw new BrainFlowError (BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR, 'invalid period');
        }
        const len = Math.trunc(data.length / period)
        const output = [...new Array (len).fill(0)];
        const res = DataHandlerDLL.getInstance().performDownsampling(
            data, data.length, period, aggOperation, output);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not perform downsampling');
        }
        return output;
    }

    public static performWaveletDenoising(data: number[], wavelet: WaveletTypes,
        decompositionLevel: number, waveletDenoising = WaveletDenoisingTypes.SURESHRINK,
        threshold = ThresholdTypes.HARD, extensionType = WaveletExtensionTypes.SYMMETRIC,
        noiseLevel = NoiseEstimationLevelTypes.FIRST_LEVEL)
    {
        const res = DataHandlerDLL.getInstance().performWaveletDenoising(data, data.length, wavelet,
            decompositionLevel, waveletDenoising, threshold, extensionType, noiseLevel);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not perform denosing');
        }
    }

    public static getOxygenLevel(ppgIr: number[], ppgRed: number[], samplingRate: number,
        coef1 = 1.5958422, coef2 = -34.6596622, coef3 = 112.6898759): number
    {
        const output = [0];
        const res = DataHandlerDLL.getInstance().getOxygenLevel(
            ppgIr, ppgRed, ppgIr.length, samplingRate, coef1, coef2, coef3, output);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not calculate oxygen level');
        }
        return output[0];
    }

    public static getHeartRate(
        ppgIr: number[], ppgRed: number[], samplingRate: number, fftSize: number): number
    {
        const output = [0];
        const res = DataHandlerDLL.getInstance().getHeartRate(
            ppgIr, ppgRed, ppgIr.length, samplingRate, fftSize, output);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not calculate heartrate');
        }
        return output[0];
    }

    public static getCustomBandPowers(data: number[][], bands: number[][], channels: number[],
        samplingRate: number, applyFilters = true): [number[], number[]]
    {
        if ((data.length == 0) || (bands.length == 0) || (channels.length == 0))
        {
            throw new BrainFlowError (BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR, 'Empty data');
        }
        var i: number;
        var j: number;
        const avgBands = [...new Array (bands.length).fill(0)];
        const stddevBands = [...new Array (bands.length).fill(0)];
        const data1D = [...new Array (data[0].length * channels.length).fill(0)];
        const startFreqs = [...new Array (bands.length).fill(0)];
        const stopFreqs = [...new Array (bands.length).fill(0)];
        for (i = 0; i < bands.length; i++)
        {
            startFreqs[i] = bands[i][0]
            stopFreqs[i] = bands[i][1]
        }
        for (i = 0; i < channels.length; i++)
        {
            for (j = 0; j < data[0].length; j++)
            {
                data1D[j + data[0].length * i] = data[channels[i]][j];
            }
        }
        const res = DataHandlerDLL.getInstance().getCustomBandPowers(data1D, channels.length,
            data[0].length, startFreqs, stopFreqs, bands.length, samplingRate,
            Number (applyFilters), avgBands, stddevBands);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not calculate band powers');
        }
        return [avgBands, stddevBands];
    }

    public static getAvgBandPowers(data: number[][], channels: number[], samplingRate: number,
        applyFilters = true): [number[], number[]]
    {
        const bands = [[2.0, 4.0], [4.0, 8.0], [8.0, 13.0], [13.0, 30.0], [30.0, 45.0]];
        return DataFilter.getCustomBandPowers(data, bands, channels, samplingRate, applyFilters);
    }
}
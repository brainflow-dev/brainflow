import koffi from 'koffi';
import _ from 'lodash';
import os from 'os';

import {
    AggOperations,
    BrainFlowError,
    BrainFlowExitCodes,
    DetrendOperations,
    FilterTypes,
    LogLevels,
    NoiseEstimationLevelTypes,
    NoiseTypes,
    ThresholdTypes,
    WaveletDenoisingTypes,
    WaveletExtensionTypes,
    WaveletTypes,
    WindowOperations
} from './brainflow.types';
import {complex} from './complex';
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
        this.libPath = `${__dirname}/../brainflow/lib`;
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

    public static getNearestPowerOfTwo(value: number): number
    {
        const output = [0];
        const res = DataHandlerDLL.getInstance().getNearestPowerOfTwo(value, output);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get nearest power');
        }
        return output[0];
    }

    public static getWindow(windowFunction: WindowOperations, len: number): number[]
    {
        const output = [...new Array (len).fill(0)];
        const res = DataHandlerDLL.getInstance().getWindow(windowFunction, len, output);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get window');
        }
        return output;
    }

    public static performFft(data: number[], windowType: WindowOperations): complex[]
    {
        var i: number;
        if (data.length % 2 != 0)
        {
            throw new BrainFlowError (
                BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR, "invalid input length");
        }
        const outputLen = Math.trunc(data.length / 2 + 1);
        const outputRe = [...new Array (outputLen).fill(0)];
        const outputIm = [...new Array (outputLen).fill(0)];
        const res = DataHandlerDLL.getInstance().performFft(
            data, data.length, windowType, outputRe, outputIm);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not calc fft');
        }
        const output: complex[] = [];
        for (i = 0; i < outputLen; i++)
        {
            output.push(new complex (outputRe[i], outputIm[i]));
        }
        return output;
    }

    public static performIfft(data: complex[]): number[]
    {
        var i: number;
        const output = [...new Array (2 * (data.length - 1)).fill(0)];
        const tempRe = [...new Array (data.length).fill(0)];
        const tempIm = [...new Array (data.length).fill(0)];
        for (i = 0; i < data.length; i++)
        {
            tempRe[i] = data[i].real;
            tempIm[i] = data[i].img;
        }
        const res = DataHandlerDLL.getInstance().performIfft(tempRe, tempIm, output.length, output);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not calc ifft');
        }

        return output;
    }

    public static getPsd(
        data: number[], samplingRate: number, windowType: WindowOperations): [number[], number[]]
    {
        if (data.length % 2 != 0)
        {
            throw new BrainFlowError (
                BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR, "invalid input length");
        }
        const outputLen = Math.trunc(data.length / 2 + 1);
        const ampls = [...new Array (outputLen).fill(0)];
        const freqs = [...new Array (outputLen).fill(0)];
        const res = DataHandlerDLL.getInstance().getPsd(
            data, data.length, samplingRate, windowType, ampls, freqs);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not calc psd');
        }
        return [ampls, freqs];
    }

    public static getPsdWelch(data: number[], nfft: number, overlap: number, samplingRate: number,
        windowType: WindowOperations): [number[], number[]]
    {
        if (data.length % 2 != 0)
        {
            throw new BrainFlowError (
                BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR, "invalid input length");
        }
        const outputLen = Math.trunc(data.length / 2 + 1);
        const ampls = [...new Array (outputLen).fill(0)];
        const freqs = [...new Array (outputLen).fill(0)];
        const res = DataHandlerDLL.getInstance().getPsdWelch(
            data, data.length, nfft, overlap, samplingRate, windowType, ampls, freqs);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not calc psd');
        }
        return [ampls, freqs];
    }

    public static getBandPower(
        psd: [number[], number[]], startFreq: number, stopFreq: number): number
    {
        const output = [0];
        const res = DataHandlerDLL.getInstance().getBandPower(
            psd[0], psd[1], psd[0].length, startFreq, stopFreq, output);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not calc band power');
        }
        return output[0];
    }

    public static performWaveletTransform(data: number[], wavelet: WaveletTypes,
        decompositionLevel: number, extension: WaveletExtensionTypes): [number[], number[]]
    {
        const waveletCoeffs = [...new Array (data.length + 2 * (40 + 1)).fill(0)];
        const lengths = [...new Array (decompositionLevel + 1).fill(0)];
        const res = DataHandlerDLL.getInstance().performWaveletTransform(
            data, data.length, wavelet, decompositionLevel, extension, waveletCoeffs, lengths);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not calc wavelet transform');
        }
        const sum = lengths.reduce((a: number, b: number) => { return a + b; }, 0);
        const coeffsNew = waveletCoeffs.slice(0, sum);
        return [coeffsNew, lengths];
    }

    public static performInverseWaveletTransform(waveletData: [number[], number[]],
        originalLen: number, wavelet: WaveletTypes, decompositionLevel: number,
        extension: WaveletExtensionTypes): number[]
    {
        const output = [...new Array (originalLen).fill(0)];
        const res = DataHandlerDLL.getInstance().performInverseWaveletTransform(waveletData[0],
            originalLen, wavelet, decompositionLevel, extension, waveletData[1], output);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not calc inverse wavelet transform');
        }
        return output;
    }

    public static restoreDataFromWaveletDetailedCoeffs(data: number[], wavelet: WaveletTypes,
        decompositionLevel: number, levelToRestore: number): number[]
    {
        const output = [...new Array (data.length).fill(0)];
        const res = DataHandlerDLL.getInstance().restoreDataFromWaveletDetailedCoeffs(
            data, data.length, wavelet, decompositionLevel, levelToRestore, output);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not restore data from wavelet');
        }
        return output;
    }

    public static detectPeaksZScore(
        data: number[], lag = 5, threshold = 3.5, influence = 0.1): number[]
    {
        const output = [...new Array (data.length).fill(0)];
        const res = DataHandlerDLL.getInstance().detectPeaksZScore(
            data, data.length, lag, threshold, influence, output);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not detect peaks');
        }
        return output;
    }

    public static performIca(data: number[][], numComponents: number,
        channels: number[]): [number[][], number[][], number[][], number[][]]
    {
        if (data.length < 1)
        {
            throw new BrainFlowError (BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR, "empty data");
        }
        var i: number;
        var j: number;
        const data1D = [...new Array (data[0].length * channels.length).fill(0)];
        const w = [...new Array (numComponents * numComponents).fill(0)];
        const k = [...new Array (channels.length * numComponents).fill(0)];
        const a = [...new Array (numComponents * channels.length).fill(0)];
        const s = [...new Array (data[0].length * numComponents).fill(0)];

        for (i = 0; i < channels.length; i++)
        {
            for (j = 0; j < data[0].length; j++)
            {
                data1D[j + data[0].length * i] = data[channels[i]][j];
            }
        }

        const res = DataHandlerDLL.getInstance().performIca(
            data1D, channels.length, data[0].length, numComponents, w, k, a, s);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not perform ica');
        }
        const wOut: number[][] = [];
        while (w.length)
            wOut.push(w.splice(0, numComponents));
        const kOut: number[][] = [];
        while (k.length)
            kOut.push(k.splice(0, channels.length));
        const aOut: number[][] = [];
        while (a.length)
            aOut.push(a.splice(0, numComponents));
        const sOut: number[][] = [];
        while (s.length)
            sOut.push(s.splice(0, data[0].length));
        return [wOut, kOut, aOut, sOut];
    }

    public static detrend(data: number[], detrendOperation: DetrendOperations)
    {
        const res = DataHandlerDLL.getInstance().detrend(data, data.length, detrendOperation);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not detrend');
        }
    }

    public static calcStddev(data: number[], startPos: number, stopPos: number): number
    {
        const output = [0];
        const res = DataHandlerDLL.getInstance().calcStddev(data, startPos, stopPos, output);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not calc stddev');
        }
        return output[0];
    }
}
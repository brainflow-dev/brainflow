import os from 'os';
import koffi from 'koffi';
import { DataHandlerCLikeFunctions as CLike, DataHandlerFunctions } from './functions.types';
import { BrainFlowExitCodes, BrainFlowError, NoiseTypes, FilterTypes } from './brainflow.types';

class DataHandlerDLL extends DataHandlerFunctions {

  private static instance: DataHandlerDLL;

  private libPath: string;
  private dllPath: string;
  private lib: koffi.IKoffiLib;

  private constructor() {
    super();
    this.libPath = `${__dirname}/lib`;
    this.dllPath = this.getDLLPath();
    this.lib = this.getLib();

    this.getRailedPercentage = this.lib.func(CLike.get_railed_percentage);
    this.performLowPass = this.lib.func(CLike.perform_lowpass);
    this.performHighPass = this.lib.func(CLike.perform_highpass);
    this.performBandPass = this.lib.func(CLike.perform_bandpass);
    this.performBandStop = this.lib.func(CLike.perform_bandstop);
    this.removeEnvironmentalNoise = this.lib.func(CLike.remove_environmental_noise);
  }

  private getDLLPath() {
    const platform = os.platform();
    const arch = os.arch();
    switch (platform) {
      case 'darwin':
        return `${this.libPath}/libDataHandler.dylib`;
      case 'win32':
        return arch === 'x64' ? `${this.libPath}/DataHandler.dll` : `${this.libPath}/DataHandler32.dll`;
      case 'linux':
        return `${this.libPath}/libDataHandler.so`;
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
      throw new BrainFlowError(BrainFlowExitCodes.GENERAL_ERROR, `${'Could not load DataHandlerDLL DLL - path://'}${this.dllPath}`);
    }
  }

  public static getInstance(): DataHandlerDLL {
    if (!DataHandlerDLL.instance) {
          DataHandlerDLL.instance = new DataHandlerDLL();
    }
    return DataHandlerDLL.instance;
  }
}

export class DataFilter {

  public static getRailedPercentage(data: number[], gain: number): number {
    const output = [0];
    const res = DataHandlerDLL.getInstance().getRailedPercentage(data, data.length, gain, output);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not get railed percentage');
    }
    return output[0];
  }

  public static performLowPass(data: number[], samplingRate: number, cutoff: number, order: number, filterType: FilterTypes, ripple: number): number {
    const output = [0];
    const res = DataHandlerDLL.getInstance().performLowPass(data, data.length, samplingRate, cutoff, order, filterType, ripple);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not apply filter');
    }
    return output[0];
  }

  public static performHighPass(data: number[], samplingRate: number, cutoff: number, order: number, filterType: FilterTypes, ripple: number): number {
    const output = [0];
    const res = DataHandlerDLL.getInstance().performHighPass(data, data.length, samplingRate, cutoff, order, filterType, ripple);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not apply filter');
    }
    return output[0];
  }

  public static performBandPass(data: number[], samplingRate: number, startFreq: number, stopFreq: number, order: number, filterType: FilterTypes, ripple: number): number {
    const output = [0];
    const res = DataHandlerDLL.getInstance().performBandPass(data, data.length, samplingRate, startFreq, stopFreq, order, filterType, ripple);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not apply filter');
    }
    return output[0];
  }

  public static performBandStop(data: number[], samplingRate: number, startFreq: number, stopFreq: number, order: number, filterType: FilterTypes, ripple: number): number {
    const output = [0];
    const res = DataHandlerDLL.getInstance().performBandStop(data, data.length, samplingRate, startFreq, stopFreq, order, filterType, ripple);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not apply filter');
    }
    return output[0];
  }

  public static removeEnvironmentalNoise(data: number[], samplingRate: number, noiseType: NoiseTypes): number {
    const output = [0];
    const res = DataHandlerDLL.getInstance().removeEnvironmentalNoise(data, data.length, samplingRate, noiseType);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new BrainFlowError(res, 'Could not apply filter');
    }
    return output[0];
  }

}

import os from 'os';
import koffi from 'koffi';
import { DataHandlerCLikeFunctions as C, DataHandlerJSFunctions as JS } from './functions.types';
import { BrainFlowExitCodes } from './brainflow.types';

class DataHandlerDLL {
  private libPath: string;
  private dllPath: string;
  private lib: koffi.IKoffiLib;

  public getRailedPercentage: JS['getRailedPercentage'];

  constructor() {
    this.libPath = `${__dirname}/lib`;
    this.dllPath = this.getDLLPath();
    this.lib = this.getLib();

    this.getRailedPercentage = this.lib.func(C.get_railed_percentage);
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
        throw new Error(`OS ${platform} is not supported.`);
    }
  }

  private getLib() {
    try {
      const lib = koffi.load(this.dllPath);
      return lib;
    } catch (err) {
      console.error(err);
      throw new Error(`${'Could not load DataHandlerDLL DLL - path://'}${this.dllPath}`);
    }
  }
}

export class DataFilter {
  public static getRailedPercentage(data: number[], gain: number): number {
    const output = [0];
    const res = new DataHandlerDLL().getRailedPercentage(data, data.length, gain, output);
    if (res !== BrainFlowExitCodes.STATUS_OK) {
      throw new Error('Could not get railed percentage');
    }
    return output[0];
  }
}

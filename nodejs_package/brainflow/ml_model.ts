import koffi from 'koffi';
import _ from 'lodash';
import * as os from 'os';

import {
    BrainFlowClassifiers,
    BrainFlowError,
    BrainFlowExitCodes,
    BrainFlowMetrics,
    IBrainFlowModelParams,
    LogLevels,
} from './brainflow.types';
import {MLModuleCLikeFunctions as CLike, MLModuleFunctions} from './functions.types';

export class BrainFlowModelParams
{
    public inputParams: IBrainFlowModelParams = {
        metric: BrainFlowMetrics.USER_DEFINED,
        classifier: BrainFlowClassifiers.ONNX_CLASSIFIER,
        file: '',
        otherInfo: '',
        outputName: '',
        maxArraySize: 8192
    };

    constructor(metric: BrainFlowMetrics, classifier: BrainFlowClassifiers,
        inputParams: Partial<IBrainFlowModelParams>)
    {
        this.inputParams = {...this.inputParams, ...inputParams };
        this.inputParams.metric = metric;
        this.inputParams.classifier = classifier;
    }

    public toJson(): string
    {
        const params: Record<string, any> = {};
        Object.keys(this.inputParams).forEach((key) => {
            params[_.snakeCase(key)] = this.inputParams[key as keyof IBrainFlowModelParams];
        });
        return JSON.stringify(params);
    }
}

class MLModuleDLL extends MLModuleFunctions
{
    private static instance: MLModuleDLL;

    private libPath: string;
    private dllPath: string;
    private lib: koffi.IKoffiLib;

    private constructor()
    {
        super ();
        this.libPath = `${__dirname}/../brainflow/lib`;
        this.dllPath = this.getDLLPath();
        this.lib = this.getLib();

        this.setLogLevelMLModule = this.lib.func(CLike.set_log_level_ml_module);
        this.setLogFileMLModule = this.lib.func(CLike.set_log_file_ml_module);
        this.logMessageMLModule = this.lib.func(CLike.log_message_ml_module);
        this.getVersionMLModule = this.lib.func(CLike.get_version_ml_module);
        this.prepare = this.lib.func(CLike.prepare);
        this.predict = this.lib.func(CLike.predict);
        this.release = this.lib.func(CLike.release);
        this.releaseAll = this.lib.func(CLike.release_all);
    }

    private getDLLPath()
    {
        const platform = os.platform();
        const arch = os.arch();
        switch (platform)
        {
            case 'darwin':
                return `${this.libPath}/libMLModule.dylib`;
            case 'win32':
                return arch === 'x64' ? `${this.libPath}/MLModule.dll` :
                                        `${this.libPath}/MLModule32.dll`;
            case 'linux':
                return `${this.libPath}/libMLModule.so`;
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
                `${'Could not load MLModule DLL - path://'}${this.dllPath}`);
        }
    }

    public static getInstance(): MLModuleDLL
    {
        if (!MLModuleDLL.instance)
        {
            MLModuleDLL.instance = new MLModuleDLL ();
        }
        return MLModuleDLL.instance;
    }
}

export class MLModel
{
    private inputJson: string;
    private input: BrainFlowModelParams;

    constructor(metric: BrainFlowMetrics, classifier: BrainFlowClassifiers,
        inputParams: Partial<IBrainFlowModelParams>)
    {
        this.input = new BrainFlowModelParams (metric, classifier, inputParams);
        this.inputJson = this.input.toJson();
    }

    // logging methods
    public static getVersion(): string
    {
        const len = [0];
        let out = ['\0'.repeat(512)];
        const res = MLModuleDLL.getInstance().getVersionMLModule(out, len, 512);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not get version info');
        }
        return out[0].substring(0, len[0]);
    }

    public static setLogLevel(logLevel: LogLevels): void
    {
        const res = MLModuleDLL.getInstance().setLogLevelMLModule(logLevel);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not set log level properly');
        }
    }

    public static setLogFile(file: string): void
    {
        const res = MLModuleDLL.getInstance().setLogFileMLModule(file);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not redirect to log file');
        }
    }

    public static logMessage(logLevel: LogLevels, message: string): void
    {
        const res = MLModuleDLL.getInstance().logMessageMLModule(logLevel, message);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not writte message');
        }
    }

    // model methods
    public prepare(): void
    {
        const res = MLModuleDLL.getInstance().prepare(this.inputJson);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not prepare model');
        }
    }

    public predict(data: number[]): number[]
    {
        const len = [0];
        const output = [...new Array (this.input.inputParams.maxArraySize).fill(0)];
        const res =
            MLModuleDLL.getInstance().predict(data, data.length, output, len, this.inputJson);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not predict');
        }
        return output.slice(0, len[0]);
    }

    public release(): void
    {
        const res = MLModuleDLL.getInstance().release(this.inputJson);
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not release model');
        }
    }

    public static releaseAll(): void
    {
        const res = MLModuleDLL.getInstance().releaseAll();
        if (res !== BrainFlowExitCodes.STATUS_OK)
        {
            throw new BrainFlowError (res, 'Could not release models');
        }
    }
}

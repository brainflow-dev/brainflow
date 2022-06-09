using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace brainflow
{
    public class MLModel
    {
        private string input_json;
        BrainFlowModelParams input_params;


        /// <summary>
        /// Create an instance of MLModel class
        /// </summary>
        /// <param name="input_params"></param>
        public MLModel (BrainFlowModelParams input_params)
        {
            this.input_json = input_params.to_json ();
            this.input_params = input_params;
        }

        /// <summary>
        /// release all classifiers
        /// </summary>
        public static void release_all ()
        {
            int res = MLModuleLibrary.release_all ();
            if (res != (int)BrainFlowExitCodes.STATUS_OK)
            {
                throw new BrainFlowError (res);
            }
        }

        /// <summary>
        /// set log level, logger is disabled by default
        /// </summary>
        /// <param name="log_level"></param>
        public static void set_log_level (int log_level)
        {
            int res = MLModuleLibrary.set_log_level_ml_module (log_level);
            if (res != (int)BrainFlowExitCodes.STATUS_OK)
            {
                throw new BrainFlowError (res);
            }
        }

        /// <summary>
        /// send your own log message to BrainFlow's logger
        /// </summary>
        /// <param name="log_level"></param>
        /// <param name="message"></param>
        public static void log_message (int log_level, string message)
        {
            int res = MLModuleLibrary.log_message_ml_module (log_level, message);
            if (res != (int)BrainFlowExitCodes.STATUS_OK)
            {
                throw new BrainFlowError (res);
            }
        }

        /// <summary>
        /// enable ML logger with level INFO
        /// </summary>
        public static void enable_ml_logger ()
        {
            set_log_level ((int)LogLevels.LEVEL_INFO);
        }

        /// <summary>
        /// disable ML logger
        /// </summary>
        public static void disable_ml_logger ()
        {
            set_log_level ((int)LogLevels.LEVEL_OFF);
        }

        /// <summary>
        /// enable ML logger with level TRACE
        /// </summary>
        public static void enable_dev_ml_logger ()
        {
            set_log_level ((int)LogLevels.LEVEL_TRACE);
        }

        /// <summary>
        /// redirect BrainFlow's logger from stderr to file
        /// </summary>
        /// <param name="log_file"></param>
        public static void set_log_file (string log_file)
        {
            int res = MLModuleLibrary.set_log_file_ml_module (log_file);
            if (res != (int)BrainFlowExitCodes.STATUS_OK)
            {
                throw new BrainFlowError (res);
            }
        }

        /// <summary>
        /// Prepare classifier
        /// </summary>
        public void prepare ()
        {
            int res = MLModuleLibrary.prepare (input_json);
            if (res != (int)BrainFlowExitCodes.STATUS_OK)
            {
                throw new BrainFlowError (res);
            }
        }

        /// <summary>
        /// Release classifier
        /// </summary>
        public void release ()
        {
            int res = MLModuleLibrary.release (input_json);
            if (res != (int)BrainFlowExitCodes.STATUS_OK)
            {
                throw new BrainFlowError (res);
            }
        }

        /// <summary>
        /// Get score of classifier
        /// </summary>
        public double[] predict (double[] data)
        {
            double[] val = new double[input_params.max_array_size];
            int[] val_len = new int[1];
            int res = MLModuleLibrary.predict (data, data.Length, val, val_len, input_json);
            if (res != (int)BrainFlowExitCodes.STATUS_OK)
            {
                throw new BrainFlowError (res);
            }
            double[] result = new double[val_len[0]];
            for (int i = 0; i < val_len[0]; i++)
            {
                result[i] = val[i];
            }
            return result;
        }

        /// <summary>
        /// get version
        /// </summary>
        /// <returns>version</returns>
        /// <exception cref="BrainFlowException"></exception>
        public static string get_version ()
        {
            int[] len = new int[1];
            byte[] str = new byte[64];
            int res = MLModuleLibrary.get_version_ml_module (str, len, 64);
            if (res != (int)BrainFlowExitCodes.STATUS_OK)
            {
                throw new BrainFlowError (res);
            }
            string version = System.Text.Encoding.UTF8.GetString (str, 0, len[0]);
            return version;
        }

    }
}

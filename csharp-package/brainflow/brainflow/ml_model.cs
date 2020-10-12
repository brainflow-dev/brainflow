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


        /// <summary>
        /// Create an instance of MLModel class
        /// </summary>
        /// <param name="input_params"></param>
        public MLModel (BrainFlowModelParams input_params)
        {
            this.input_json = input_params.to_json ();
        }

        /// <summary>
        /// set log level, logger is disabled by default
        /// </summary>
        /// <param name="log_level"></param>
        public static void set_log_level (int log_level)
        {
            int res = MLModuleLibrary.set_log_level (log_level);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
        }

        /// <summary>
        /// enable BrainFlow's logger with level INFO
        /// </summary>
        public static void enable_board_logger ()
        {
            MLModuleLibrary.set_log_level ((int)LogLevels.LEVEL_INFO);
        }

        /// <summary>
        /// disable BrainFlow's logger
        /// </summary>
        public static void disable_board_logger ()
        {
            MLModuleLibrary.set_log_level ((int)LogLevels.LEVEL_OFF);
        }

        /// <summary>
        /// enable BrainFLow's logger with level TRACE
        /// </summary>
        public static void enable_dev_board_logger ()
        {
            MLModuleLibrary.set_log_level ((int)LogLevels.LEVEL_TRACE);
        }

        /// <summary>
        /// redirect BrainFlow's logger from stderr to file
        /// </summary>
        /// <param name="log_file"></param>
        public static void set_log_file (string log_file)
        {
            int res = MLModuleLibrary.set_log_file (log_file);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
        }

        /// <summary>
        /// Prepare classifier
        /// </summary>
        public void prepare ()
        {
            int res = MLModuleLibrary.prepare (input_json);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
        }

        /// <summary>
        /// Release classifier
        /// </summary>
        public void release ()
        {
            int res = MLModuleLibrary.release (input_json);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
        }

        /// <summary>
        /// Get score of classifier
        /// </summary>
        public double predict (double[] data)
        {
            double[] val = new double[1];
            int res = MLModuleLibrary.predict (data, data.Length, val, input_json);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
            return val[0];
        }

    }
}

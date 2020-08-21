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

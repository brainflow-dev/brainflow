using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace brainflow
{
    public class MLModel
    {
        private int metric;
        private int classifier;

        /// <summary>
        /// Create an instance of MLModel class
        /// </summary>
        /// <param name="metric"></param>
        /// <param name="classifier"></param>
        public MLModel (int metric, int classifier)
        {
            this.metric = metric;
            this.classifier = classifier;
        }

        /// <summary>
        /// Prepare classifier
        /// </summary>
        public void prepare ()
        {
            int res = MLModuleLibrary.prepare (metric, classifier);
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
            int res = MLModuleLibrary.release (metric, classifier);
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
            int res = MLModuleLibrary.predict (data, data.Length, val, metric, classifier);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
            return val[0];
        }

    }
}

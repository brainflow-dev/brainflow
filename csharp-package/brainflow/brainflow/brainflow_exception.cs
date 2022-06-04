using System;


namespace brainflow
{
    /// <summary>
    /// BrainFlowError class to notify about errors
    /// </summary>
    public class BrainFlowError : Exception
    {
        /// <summary>
        /// exit code returned from low level API
        /// </summary>
        public int exit_code;
        public BrainFlowError (int code) : base (String.Format ("{0}:{1}", Enum.GetName (typeof (BrainFlowExitCodes), code), code))
        {
            exit_code = code;
        }
    }
}

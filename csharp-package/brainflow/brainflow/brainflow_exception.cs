using System;


namespace brainflow
{
    /// <summary>
    /// BrainFlowException class to notify about errors
    /// </summary>
    public class BrainFlowException : Exception
    {
        /// <summary>
        /// exit code returned from low level API
        /// </summary>
        public int exit_code;
        public BrainFlowException (int code) : base (String.Format ("{0}:{1}", Enum.GetName (typeof (CustomExitCodes), code), code))
        {
            exit_code = code;
        }
    }
}

using System;

namespace brainflow
{
    public class BrainFlowException : Exception
    {
        public int exit_code;
        public BrainFlowException (int code) : base (String.Format ("{0}:{1}", Enum.GetName (typeof (CustomExitCodes), code), code))
        {
            exit_code = code;
        }
    }
}

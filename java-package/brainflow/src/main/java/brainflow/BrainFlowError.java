package brainflow;

/**
 * BrainFlowError exception to notify about errors
 */
@SuppressWarnings ("serial")
public class BrainFlowError extends Exception
{
    public String msg;
    /**
     * exit code returned from low level API
     */
    public int exit_code;

    public BrainFlowError (String message, int ec)
    {
        super (message + ":" + brainflow.ExitCode.string_from_code (ec));
        exit_code = ec;
        msg = message;
    }
}

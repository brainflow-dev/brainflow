package brainflow;

public class BrainFlowError extends Exception
{
    public String msg;
    public int exit_code;

    public BrainFlowError (String message, int ec)
    {
        super (message + ":" + brainflow.ExitCode.string_from_code (ec));
        exit_code = ec;
        msg = message;
    }
}

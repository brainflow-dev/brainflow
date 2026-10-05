package brainflow;

import java.nio.file.Paths;

/** Run with an ONNX classifier returning [relaxed, focused] and rebuilt MLModule. */
public class MLBindingValidationRegression
{
    private static void rejectsCapacity (int capacity)
    {
        BrainFlowModelParams params = new BrainFlowModelParams (0, 0);
        params.max_array_size = capacity;
        try
        {
            new MLModel (params);
        } catch (IllegalArgumentException expected)
        {
            return;
        }
        throw new AssertionError ("Invalid output capacity was accepted");
    }

    public static void main (String[] args) throws Exception
    {
        rejectsCapacity (0);
        rejectsCapacity (-1);
        if (args.length != 1)
        {
            throw new IllegalArgumentException ("Supply the path to logreg_mindfulness.onnx");
        }
        BrainFlowModelParams params = new BrainFlowModelParams (2, 2);
        params.file = Paths.get (args[0]).toAbsolutePath ().toString ();
        params.max_array_size = 2;
        MLModel model = new MLModel (params);
        model.prepare ();
        try
        {
            params.max_array_size = 1;
            double[] result = model.predict (new double[] {0.2, 0.2, 0.2, 0.2, 0.2});
            if (result.length != 2 || !Double.isFinite (result[0]) || !Double.isFinite (result[1]) ||
                    result[0] < 0 || result[1] < 0 || Math.abs (result[0] + result[1] - 1) > 1e-6)
            {
                throw new AssertionError ("Invalid probabilities after mutating model params");
            }
        } finally
        {
            model.release ();
        }
        System.out.println ("ML capacity snapshot validation passed.");
    }
}

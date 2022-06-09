package brainflow;

import com.google.gson.Gson;

/**
 * describe model parameters
 */
public class BrainFlowModelParams
{
    public int metric;
    public int classifier;
    public String file;
    public String other_info;
    public String output_name;
    public int max_array_size;

    public BrainFlowModelParams (int metric, int classifier)
    {
        this.metric = metric;
        this.classifier = classifier;
        this.file = "";
        this.other_info = "";
        this.output_name = "";
        this.max_array_size = 8192;
    }

    public BrainFlowModelParams (BrainFlowMetrics metric, BrainFlowClassifiers classifier)
    {
        this.metric = metric.get_code ();
        this.classifier = classifier.get_code ();
        this.file = "";
        this.other_info = "";
        this.output_name = "";
        this.max_array_size = 8192;
    }

    public int get_metric ()
    {
        return metric;
    }

    public void set_metric (int metric)
    {
        this.metric = metric;
    }

    public void set_metric (BrainFlowMetrics metric)
    {
        this.metric = metric.get_code ();
    }

    public int get_classifier ()
    {
        return classifier;
    }

    public void set_classifier (BrainFlowClassifiers classifier)
    {
        this.classifier = classifier.get_code ();
    }

    public void set_classifier (int classifier)
    {
        this.classifier = classifier;
    }

    public String get_file ()
    {
        return file;
    }

    public void set_file (String file)
    {
        this.file = file;
    }

    public String get_other_info ()
    {
        return other_info;
    }

    public void set_other_info (String other_info)
    {
        this.other_info = other_info;
    }

    public String get_output_name ()
    {
        return output_name;
    }

    public void set_output_name (String output_name)
    {
        this.output_name = output_name;
    }

    public int get_max_array_size ()
    {
        return max_array_size;
    }

    public void set_max_array_size (int max_array_size)
    {
        this.max_array_size = max_array_size;
    }

    public String to_json ()
    {
        return new Gson ().toJson (this);
    }

}
